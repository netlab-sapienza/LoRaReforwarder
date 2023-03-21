#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>

#include "server/server.hpp"
#include "common/common.hpp"
using namespace lorareforwarder;

#define SERVER_PORT 8888

int main(int argc, char const *argv[]) {
    server::LoRaServer server(SERVER_PORT);
}