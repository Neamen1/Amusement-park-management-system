#include <iostream>
#include "server.cpp"
//#include <libpq-fe.h> //test db conn

#include <chrono>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;

// Benchmark function
void benchmark(const std::vector<std::string> directories, const std::vector<size_t>& fileCounts, const std::vector<size_t>& threadCounts, const std::string& output_file) {
    std::ofstream output(output_file);
    if (!output.is_open()) {
        std::cerr << "Failed to open output file: " << output_file << std::endl;
        return;
    }

    // Write CSV header
    output << "num_files,num_threads,time_taken_addFiles_ms,time_taken_buildIndex_ms" << std::endl;

    std::string directory = directories[2];
    Server server(1, directories); //  із 1 потоком

    for (size_t threadCount : threadCounts) {
        for (size_t fileCount : fileCounts) {
            auto startReadFilenames = std::chrono::high_resolution_clock::now();

            server.addFilenamesFromDirectory(directory, fileCount, threadCount);

            auto endReadFilenames = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsedReadFilenames = endReadFilenames - startReadFilenames;


            auto start = std::chrono::high_resolution_clock::now();

            // Build the index using the specified number of threads
            server.index.buildIndexParallel(server.lastModifiedMutex, server.lastModified, threadCount);

            // Stop timer
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;

            server.index.clearIndex();
            server.lastModified.clear();

            // Output results
            output << fileCount << "," << threadCount << "," << elapsedReadFilenames.count() << "," << elapsed.count() << std::endl;
            std::cout << fileCount<< ", " << threadCount << ", " << elapsedReadFilenames.count() << "," << elapsed.count() << "\n";

        }
    }
    output.close();
}

//int main() {
//    std::vector<std::string> directories = {
//                "G:\\OneDrive\\disk save\\uni\\7sem onedrive\\pis + kurs\\main fold\\inv index module\\Inverted index system\\x64\\Debug\\aclImdb\\test\\neg",
//                "G:\\OneDrive\\disk save\\uni\\7sem onedrive\\pis + kurs\\main fold\\inv index module\\Inverted index system\\x64\\Debug\\aclImdb\\test\\pos",
//                "G:\\OneDrive\\disk save\\uni\\7sem onedrive\\pis + kurs\\main fold\\inv index module\\Inverted index system\\x64\\Debug\\aclImdb\\train\\neg",
//                "G:\\OneDrive\\disk save\\uni\\7sem onedrive\\pis + kurs\\main fold\\inv index module\\Inverted index system\\x64\\Debug\\aclImdb\\train\\pos",
//                "G:\\OneDrive\\disk save\\uni\\7sem onedrive\\pis + kurs\\main fold\\inv index module\\Inverted index system\\x64\\Debug\\aclImdb\\train\\unsup"
//    };
//
//    // Configure file counts and thread counts
//    std::vector<size_t> fileCounts = { 100, 500, 1000, 2000, 5000 };
//    std::vector<size_t> threadCounts = {1, 2, 4, 6, 8, 16 };
//
//    // Run the benchmark
//    benchmark(directories, fileCounts, threadCounts, "G:\\OneDrive\\disk save\\uni\\7sem onedrive\\pis + kurs\\main fold\\stress_test\\perf_data train-neg.csv");
//
//    return 0;
//}



int main() {
    std::vector<std::string> directories = {
        "G:\\OneDrive\\disk save\\uni\\7sem onedrive\\pis + kurs\\main fold\\inv index module\\Inverted index system\\x64\\Debug\\aclImdb\\test\\neg",
        "G:\\OneDrive\\disk save\\uni\\7sem onedrive\\pis + kurs\\main fold\\inv index module\\Inverted index system\\x64\\Debug\\aclImdb\\test\\pos",
        "G:\\OneDrive\\disk save\\uni\\7sem onedrive\\pis + kurs\\main fold\\inv index module\\Inverted index system\\x64\\Debug\\aclImdb\\train\\neg",
        "G:\\OneDrive\\disk save\\uni\\7sem onedrive\\pis + kurs\\main fold\\inv index module\\Inverted index system\\x64\\Debug\\aclImdb\\train\\pos",
        "G:\\OneDrive\\disk save\\uni\\7sem onedrive\\pis + kurs\\main fold\\inv index module\\Inverted index system\\x64\\Debug\\aclImdb\\train\\unsup"
    };

    Server server(4, directories); // Пул потоків із 4 потоків
    server.run("127.0.0.1", 8080);
    return 0;
}
