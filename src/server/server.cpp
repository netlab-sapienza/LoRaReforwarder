#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <future>
#include <cerrno>
#include <cstring>

#include <thread>
#include <chrono>

#include "packet/packetParser.hpp"
#include "socket/server.hpp"
#include "server.hpp"

namespace lorareforwarder::server {

    LoRaServer::~LoRaServer() = default;

    void LoRaServer::init() {
        netserver_host = common::hostToIp(netserver_hostname);

        UDPServer udpServer;

        UDPSocket socket(true, [](int errorCode, std::string errorMessage) {
            common::handle_error_en(errorCode, errorMessage);
        });

        udpServer.onRawMessageReceived = [&](const uint8_t* message, int length, std::string ipv4, uint16_t port)  {
            if (debug) std::cout << ipv4 << ":" << port << " => " << message << "(" << length << ")" << " => " << packet::LoRaPacketParser::getPacketType(message[3]) << std::endl;
            socket.SendTo(message, length, netserver_host, netserver_port);

            if(packet::LoRaPacketParser::getPacketType(message[3]) == "PKT_PUSH_DATA") {
                packet::LoRaPacket* pushpacket = packet::LoRaPacketParser::parse(message, length, debug);
                std::async(std::launch::async, [&]() {
                    database::Database::insertPushDataPacketStatic(database, pushpacket);
                });
            }
            
            socket.onRawMessageReceived = [&](const uint8_t* servermessage, int serverlength, std::string serveripv4, uint16_t serverport) {
                if (debug) std::cout << "SERVER " << serveripv4 << ":" << serverport << " => " << servermessage << "(" << serverlength << ")" << " => " << packet::LoRaPacketParser::getPacketType(servermessage[3]) << std::endl;                
                udpServer.SendTo(servermessage, serverlength, ipv4, port);

                if(packet::LoRaPacketParser::getPacketType(servermessage[3]) == "PKT_PULL_RESP") {
                    packet::LoRaPacket* pullpacket = packet::LoRaPacketParser::parse(servermessage, serverlength, debug);
                    std::async(std::launch::async, [&]() {
                        database::Database::insertPullDataPacketStatic(database, pullpacket);
                    });
                }
            };
        };
        
        std::cout << "Forwarder listening on UDP Port: " << port << std::endl;
        udpServer.Bind(port, [](int errorCode, std::string errorMessage) {
            common::handle_error_en(errorCode, errorMessage);
        });

        while(1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}