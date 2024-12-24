#include <iostream>
#include <asio.hpp>
#include "server.cpp"

int main() {
    Server server(4); // Пул потоків із 4 потоків
    server.run("127.0.0.1", 8080);
    return 0;
}
