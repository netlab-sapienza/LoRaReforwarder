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

#define SERVER_PORT 8888

#define NETWORK_SERVER_ADDR "localhost" // "63.34.215.128"
#define NETWORK_SERVER_PORT 1700

int main(int argc, char const *argv[]) {
    database::Database database("10.6.0.2", "lorareforwarder", "lorareforwarderpwd", "lorareforwarder", 3306);
    server::LoRaServer server(SERVER_PORT, database, NETWORK_SERVER_ADDR, NETWORK_SERVER_PORT);
}