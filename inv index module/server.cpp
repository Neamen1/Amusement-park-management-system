#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <asio.hpp>
#include "threadpool.cpp"
#include "InvertedIndex.cpp"

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
            std::cout << response.str() << std::endl;
            return response.str();
        }
        else if (action == "UPDATE") {
            index.clearIndex();
            // Тут повинен бути код для повторного індексації з PostgreSQL
            return "OK|Index updated\n";
        }

        return "ERROR|Unknown command\n";
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
