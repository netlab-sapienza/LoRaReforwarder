#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <future>
#include <errno.h>
#include <string>

#include "packet/packetParser.hpp"
#include "socket/server.hpp"
#include "server.hpp"

namespace lorareforwarder::server {
    LoRaServer::LoRaServer(uint16_t port, database::Database& _database) : database {_database} {
        database = _database;
        init(port);
    }

    LoRaServer::~LoRaServer() = default;

    void LoRaServer::init(uint16_t port) {
        UDPServer udpServer;

        UDPSocket socket([](int errorCode, std::__1::string errorMessage) {
            std::cout << errorCode << " : " << errorMessage << std::endl;
        });

        udpServer.onRawMessageReceived = [&](const uint8_t* message, int length, std::string ipv4, uint16_t port)  {
            std::cout << ipv4 << ":" << port << " => " << message << "(" << length << ")" << " => " << packet::LoRaPacketParser::getPacketType(message[3]) << std::endl;
            socket.SendTo(message, length, "localhost", 1700);

            packet::LoRaPacket* pack = packet::LoRaPacketParser::parse(message, length);
            if(packet::LoRaPacketParser::getPacketType(message[3]) == "PKT_PUSH_DATA") 
                std::async(std::launch::async, [&]() {
                    database::Database::insertPushDataPacketStatic(database, pack);
                });

            socket.onRawMessageReceived = [&](const uint8_t* servermessage, int serverlength, std::string serveripv4, uint16_t serverport) {
                std::cout << "SERVER " << serveripv4 << ":" << serverport << " => " << servermessage << "(" << serverlength << ")" << " => " << packet::LoRaPacketParser::getPacketType(servermessage[3]) << std::endl;                
                udpServer.SendTo(servermessage, serverlength, ipv4, port);

                if(packet::LoRaPacketParser::getPacketType(servermessage[3]) == "PKT_PULL_RESP") {
                    packet::LoRaPacket* pullpacket = packet::LoRaPacketParser::parse(servermessage, serverlength);
                    std::async(std::launch::async, [&]() {
                        database::Database::insertPullDataPacketStatic(database, pullpacket);
                    });
                }
            };
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
}