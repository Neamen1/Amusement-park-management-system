#include <iostream>
#include <asio.hpp>
#include "server.cpp"
//#include <libpq-fe.h> //test db conn

int main() {
    /*std::vector<std::string> directories = {
        "aclImdb/test/neg",
        "aclImdb/test/pos",
        "aclImdb/train/neg",
        "aclImdb/train/pos",
        "aclImdb/train/unsup"
    };*/

    std::vector<std::string> directories = {
        "G:\\OneDrive\\disk save\\uni\\7sem onedrive\\pis + kurs\\main fold\\inv index module\\Inverted index system\\x64\\Debug\\aclImdb\\test\\neg"
    };

    Server server(4, directories); // Пул потоків із 4 потоків
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
