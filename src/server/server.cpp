#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <string>

#include "packet/packet.hpp"
#include "packet/packetParser.hpp"
#include "socket/server.hpp"
#include "server.hpp"

namespace lorareforwarder::server {
    LoRaServer::LoRaServer(uint16_t port) {
        init(port);
    }

    LoRaServer::~LoRaServer() = default;

    void LoRaServer::init(uint16_t port) {
        UDPServer udpServer;

        UDPSocket socket(false, [](int errorCode, std::__1::string errorMessage) {
            std::cout << errorCode << " : " << errorMessage << std::endl;
        });

        // AGGIUNGERE CONTROLLO DI CONSISTENZA PACCHETTO
        udpServer.onRawMessageReceived = [&](const uint8_t* message, int length, std::string ipv4, uint16_t port) {
            std::cout << ipv4 << ":" << port << " => " << message << "(" << length << ")" << std::endl;
            socket.SendTo(message, length, "localhost", 1700);

            bool exit = false;
            // AGGIUNGERE CONTROLLO DI CONSISTENZA PACCHETTO
            socket.onRawMessageReceived = [&](const uint8_t* servermessage, int serverlength, std::string serveripv4, uint16_t serverport) {
                std::cout << "SERVER " << serveripv4 << ":" << serverport << " => " << servermessage << "(" << serverlength << ")" << std::endl;                
                udpServer.SendTo(servermessage, serverlength, ipv4, port);

                // AGGIUNGERE CONTROLLO SUL PULL_DATA, DEVE CHIUDERE IL THREAD SOLO QUANDO HA RICEVUTO I DATI CHE RICHIEDEVA
                if (servermessage[3] != 2)
                    exit = true;
            };

            while(!exit) { }

            socket.onRawMessageReceived = NULL;
            return;
        };
        
        udpServer.Bind(port, [](int errorCode, std::string errorMessage) {
            std::cout << errorCode << " : " << errorMessage << std::endl;
        });

        std::string input;
        getline(std::cin, input);
        while (input != "exit") {
            std::getline(std::cin, input);
        }
    }

    void* LoRaServer::connection_handler(void* thread_data) {

    }
}