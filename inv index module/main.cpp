#include <iostream>
#include <asio.hpp>
#include "server.cpp"
//#include <libpq-fe.h> //test db conn

int main() {
    Server server(4); // Пул потоків із 4 потоків
    server.run("127.0.0.1", 8080);
    return 0;


    //test db conn
    /*PGconn* conn = PQconnectdb("user=admin password=1221 port=5432 dbname=amusement_database host=localhost");
    if (PQstatus(conn) == CONNECTION_OK) {
        std::cout << "Connected to the database successfully!" << std::endl;
    }
    else {
        std::cerr << "Connection failed: " << PQerrorMessage(conn) << std::endl;
    }
    PQfinish(conn);
    return 0;*/
}
