#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <asio.hpp>
#include "threadpool.cpp"
#include "InvertedIndex.cpp"
#include <fstream>
#include <libpq-fe.h>

using asio::ip::tcp;

class Server {
private:
    InvertedIndex index;
    ThreadPool pool;

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

        return "ERROR|Unknown command\n";
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

public:
    Server(size_t threads) : pool(threads) {}
    Server() : pool(4) {}

    void run(const std::string& host, int port) {
        try {
            asio::io_context ioContext;
            tcp::acceptor acceptor(ioContext, tcp::endpoint(tcp::v4(), port));

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
};
