#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>

#include "dotenv.h"

#include "server/server.hpp"
#include "database/database.hpp"
using namespace lorareforwarder;

int main(int argc, char const *argv[]) {
    auto& dotenv = dotenv::env.load_dotenv(".env");

    if(dotenv["MARIADB_SERVER_ADDR"] == "" || dotenv["MARIADB_SERVER_PORT"] == ""
        || dotenv["MARIADB_SERVER_USERNAME"] == "" || dotenv["MARIADB_SERVER_PWD"] == "" || dotenv["MARIADB_SERVER_DBNAME"] == ""
        || dotenv["NETWORK_SERVER_ADDR"] == "" || dotenv["NETWORK_SERVER_PORT"] == "" || dotenv["SERVER_PORT"] == "") {
        std::cerr << "Please set env variables file correctly!" << std::endl;
        return -1;
    }

    uint16_t serverPort = static_cast<uint16_t>(std::stoul(dotenv["SERVER_PORT"]));
    uint16_t networkServerPort = static_cast<uint16_t>(std::stoul(dotenv["NETWORK_SERVER_PORT"]));
    uint32_t mariadbServerPort = std::stoul(dotenv["MARIADB_SERVER_PORT"]);
    bool debug = dotenv["DEBUG"] == "true" ? true : false;

    // DEBUG
    // if(debug) {
    //     std::cout << dotenv["MARIADB_SERVER_ADDR"] << " " << dotenv["MARIADB_SERVER_USERNAME"] << " " << dotenv["MARIADB_SERVER_PWD"] << " " << dotenv["MARIADB_SERVER_DBNAME"] << " " << mariadbServerPort << std::endl;
    //     std::cout << dotenv["NETWORK_SERVER_ADDR"] << " " << networkServerPort << std::endl;
    // }

    database::Database database(dotenv["MARIADB_SERVER_ADDR"], dotenv["MARIADB_SERVER_USERNAME"], dotenv["MARIADB_SERVER_PWD"], dotenv["MARIADB_SERVER_DBNAME"], mariadbServerPort, debug);
    server::LoRaServer server(serverPort, database, dotenv["NETWORK_SERVER_ADDR"], networkServerPort, debug);

    server.init();
}
