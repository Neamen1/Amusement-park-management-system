#pragma warning(disable : 4996)

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <asio.hpp>
#include "threadpool.cpp"
#include "InvertedIndex.cpp"
#include <fstream>
#include <libpq-fe.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>

using asio::ip::tcp;

class Server {
private:
public:
    InvertedIndex index;
    ThreadPool pool;

    std::vector<std::string> monitoredDirectories;
    std::atomic<bool> running; // To control scheduler's lifecycle
    std::thread schedulerThread;
    std::mutex schedulerMutex;
    std::condition_variable schedulerCv;

    std::unordered_map<std::filesystem::path, std::filesystem::file_time_type> lastModified; //contains pairs of path:file_time_type of files to be tracked

    std::mutex lastModifiedMutex;

    void handleClient(tcp::socket socket) {
        try {
            char data[1024];
            size_t length = socket.read_some(asio::buffer(data));
            std::string request(data, length);

            std::string response = processRequest(request);
            asio::write(socket, asio::buffer(response));
        }
        catch (std::exception& e) {
            std::cerr << "Error handling client: " << e.what() << std::endl;
        }
    }

    std::string processRequest(const std::string& request) {
        std::istringstream stream(request);
        std::string action, term;
        stream >> action;

        if (action == "SEARCH") {
            stream >> term;
            auto results = index.search(term);
            std::ostringstream response;
            response << "OK|";
            for (const auto& docId : results) {
                response << docId << ",";
            }
            return response.str();
        }
        else if (action == "SEARCH_PHRASE") {
            std::string phrase;
            stream >> std::ws;
            std::getline(stream, phrase, '\n');
            auto results = index.searchPhrase(phrase);
            std::ostringstream response;
            response << "OK|";
            for (const auto& docId : results) {
                response << docId << ",";
            }
            return response.str();
        }
        else if (action == "CLEAR_INDEX") {
            std::string connInfo;
            stream >> connInfo;

            index.clearIndex();
            return "OK|Index cleared\n";
        }
        else if (action == "INDEX_DB") {
            std::string connInfo;
            stream >> std::ws;
            std::getline(stream, connInfo, '\n');
            if (indexFromPostgres(connInfo)) {
                return "OK|Database indexed\n";
            };
            return "ERROR|Failed database connection or error fetching data\n";
        }
        else if (action == "ADD_FILE") {
            std::string filePath;
            stream >> std::ws;
            std::getline(stream, filePath, '\n');

            std::ifstream file(filePath);
            if (!file.is_open()) {
                return "ERROR|Cannot open file\n"+ filePath;
            }
            std::ostringstream buffer;
            buffer << file.rdbuf();
            index.addDocument(/*docId=*/std::hash<std::string>{}(filePath), buffer.str());
            return "OK|File indexed\n";
        }
        else if (action == "SAVE_INDEX") {
            std::string filePath;
            stream >> std::ws;
            std::getline(stream, filePath, '\n');
            index.saveIndex(filePath);
            return "OK|Index saved\n";
        }
        else if (action == "LOAD_INDEX") {
            std::string filePath;
            stream >> std::ws;
            std::getline(stream, filePath, '\n');
            index.loadIndex(filePath);
            return "OK|Index loaded\n";
        }
        else if (action == "PRINT_INDEX") {
            std::string term;
            stream >> std::ws;
            stream >> term; // Спроба прочитати термін із запиту
            return index.printIndex(term);
        }
        else if (action == "addDocsFromDirectory") {
            std::string params;
            stream >> std::ws; // Skip whitespace
            std::getline(stream, params, '\n'); // Read the comma-separated parameters

            // Parse the comma-separated values
            size_t numThreads = 4; // Default

            std::string directoryPath;
            try {
                size_t pos1 = params.find(',');

                if (pos1 != std::string::npos) {
                    numThreads = std::stoul(params.substr(0, pos1));
                    directoryPath = params.substr(pos1 + 1);
                }
                else {
                    return "ERROR|Invalid parameters format. Expected numThreads,dirPath, got:" + params + "\n";
                }
            }
            catch (const std::exception& e) {
                return std::string("ERROR|Failed to parse parameters: ") + e.what() + "\n";
            }

            // Validate parsed values
            if (numThreads == 0) {
                return "ERROR|Invalid parameter values. Ensure numThreads > 0\n";
            }

            addFilenamesFromDirectory(directoryPath, 250, 2);       //fills lastModified with regular file pathes from directory
            index.buildIndexParallel(lastModifiedMutex, lastModified, 4);
            return "OK|Indexing started\n";
        }

        return "ERROR|Unknown command\n";
    }

    void runScheduler() {
        while (running.load()) {
            {
                std::unique_lock<std::mutex> lock(schedulerMutex);
                schedulerCv.wait_for(lock, std::chrono::minutes(2)); // Run every 5 minutes
                if (!running.load()) break;
            }
            monitorDirectories(); // Відслідковуємо нові та змінені файли
        }
    }

    bool indexFromPostgres(const std::string& connInfo) {
        PGconn* conn = PQconnectdb(connInfo.c_str());

        if (PQstatus(conn) != CONNECTION_OK) {
            std::cerr << "Connection to PostgreSQL failed: " << PQerrorMessage(conn) << std::endl;
            PQfinish(conn);
            return false;
        }

        PGresult* res = PQexec(conn, "SELECT id, name, description FROM attractions");
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            std::cerr << "Failed to fetch data: " << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            PQfinish(conn);
            return false;
        }

        int rows = PQntuples(res);
        for (int i = 0; i < rows; ++i) {
            int docId = std::stoi(PQgetvalue(res, i, 0));
            std::string name = PQgetvalue(res, i, 1);
            std::string description = PQgetvalue(res, i, 2);
            index.addDocument(docId, name + " " + description);
        }

        PQclear(res);
        PQfinish(conn);
        return true;
    }

    //fills lastModified with regular file pathes from directory
    void addFilenamesFromDirectory(const std::string& directory, size_t max_files, size_t numThreads) {

        ThreadPool threadPoolFiles(numThreads);
        size_t count = 0;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if(std::filesystem::is_regular_file(entry)){
                if (++count >= max_files) break;

                threadPoolFiles.enqueue([this, path = entry.path()]() {
                    std::ifstream file(path);
                    if (file.is_open()) {
                        std::unique_lock lock(lastModifiedMutex);
                        lastModified[path] = std::filesystem::last_write_time(path);
                    }
                });
            }
        }
    }

    // fills lastModified map {path:file_time_type} with file pathes from specified directory within spceified index range (index taken from file name)
    void addFilterFilenamesInDirectory(const std::string& directory,
        size_t numThreads, const std::pair<size_t, size_t>& indexRanges) {

        {
            ThreadPool threadPoolFilterFiles(numThreads);
            int file_amount = indexRanges.second - indexRanges.first;
            for (const auto& entry : fs::directory_iterator(directory)) {
                if (file_amount <= 0)
                    break;

                const std::string filename = entry.path().filename().string();

                // Extract the sequence number before '_'
                size_t underscorePos = filename.find('_');
                if (underscorePos == std::string::npos) {
                    continue; // Skip files with unexpected format
                }

                size_t sequenceNum;
                try {
                    sequenceNum = std::stoull(filename.substr(0, underscorePos));
                }
                catch (const std::exception&) {
                    continue; // Skip files with invalid sequence number
                }
                // Check if the sequence number is within the desired range
                if (sequenceNum >= indexRanges.first && sequenceNum < indexRanges.second) {
                    --file_amount;
                    threadPoolFilterFiles.enqueue([this, path=entry.path()]() {
                        std::ifstream file(path);
                        if (file.is_open()) {
                            std::unique_lock lock(lastModifiedMutex);
                            lastModified[path]= std::filesystem::last_write_time(path);
                        }
                    });
                }
            }
            
        }

    }

    void monitorDirectories() {
        // lambda to execute only once to add files from directories
        static bool once = [this]() {
            //write filenames into lastModified to track files (within pair of indexes) from directory
            for (const auto& dir : monitoredDirectories) {
                //pair 4000, 4250 according to task; and 16000,17000 for unsup
                if (dir == "G:\\OneDrive\\disk save\\uni\\7sem onedrive\\pis + kurs\\main fold\\inv index module\\Inverted index system\\x64\\Debug\\aclImdb\\train\\unsup") {
                    addFilterFilenamesInDirectory(dir, 4, std::make_pair(16000, 17000));
                }
                else {
                    addFilterFilenamesInDirectory(dir, 4, std::make_pair(4000, 4250));
                }
            }
            index.buildIndexParallel(lastModifiedMutex, lastModified, 4);
            return true;
            } ();

        //update tracked files
        try {
            std::unique_lock lock(lastModifiedMutex);

            for (const auto& [path, lastMod] : lastModified) {
                auto lastWriteTime = std::filesystem::last_write_time(path);

                // Якщо файл змінений
                if (lastMod != lastWriteTime) {
                    std::ifstream file(path);
                    if (file.is_open()) {
                        std::ostringstream buffer;
                        buffer << file.rdbuf();
                        index.addDocument(/*docId=*/std::hash<std::string>{}(path.string()), buffer.str());
                        lastModified[path] = lastWriteTime; // Оновлюємо час модифікації
                        //std::cout << "Indexed file: " << path << std::endl;
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error monitoring: " << e.what() << std::endl;
        }
        
    }


    Server(size_t threads, const std::vector<std::string>& dirs) 
        : pool(threads), monitoredDirectories(dirs), running(true) {}

    ~Server() {
        stopScheduler();
    }

    void run(const std::string& host, int port) {
        schedulerThread = std::thread(&Server::runScheduler, this);

        try {
            asio::io_context ioContext;
            tcp::acceptor acceptor(ioContext, tcp::endpoint(tcp::v4(), port));

            monitorDirectories(); // Відслідковуємо нові та змінені файли

            while (true) {
                auto socket = std::make_shared<tcp::socket>(ioContext); // Use shared_ptr for socket
                acceptor.accept(*socket);

                pool.enqueue([this, socket]() mutable { // Capture shared_ptr instead of raw socket. Capturing the shared pointer (socket) by value ensures it stays alive even after the run function's scope exits.
                    handleClient(std::move(*socket));
                });
            }
        }
        catch (std::exception& e) {
            std::cerr << "Error starting server: " << e.what() << std::endl;
        }
    }

    void stopScheduler() {
        running.store(false);
        schedulerCv.notify_all();
        if (schedulerThread.joinable()) {
            schedulerThread.join();
        }
    }
};
