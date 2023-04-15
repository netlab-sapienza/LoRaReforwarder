#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "server/server.hpp"
#include "database/database.hpp"
#include "common/common.hpp"
using namespace lorareforwarder;

#define SERVER_PORT 1700

#define NETWORK_SERVER_ADDR "63.34.215.128" // "63.34.215.128"
#define NETWORK_SERVER_PORT 1700

#define MARIADB_SERVER_ADDR "172.18.0.2"
#define MARIADB_SERVER_PORT 3306
#define MARIADB_USERNAME "lorareforwarder"
#define MARIADB_PWD "lorareforwarderpwd"
#define MARIADB_SERVER_DB "lorareforwarder"

int main(int argc, char const *argv[]) {
    database::Database database(MARIADB_SERVER_ADDR, MARIADB_USERNAME, MARIADB_PWD, MARIADB_SERVER_DB, MARIADB_SERVER_PORT);
    
    server::LoRaServer server(SERVER_PORT, database, NETWORK_SERVER_ADDR, NETWORK_SERVER_PORT);
    server.init();
}