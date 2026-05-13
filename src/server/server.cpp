#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <future>
#include <csignal>
#include <sstream>
#include <string>
#include <condition_variable>
#include <memory>

#include <thread>
#include <chrono>

#include "../packet/packetParser.hpp"
#include "../common/common.hpp"
#include "socket/server.hpp"
#include "gatewayRegistry.hpp"
#include "server.hpp"

namespace lorareforwarder::server {

    static volatile sig_atomic_t g_running = 1;
    static std::shared_ptr<BaseSocket> g_shutdownSocket;

    void signalHandler(int) {
        g_running = 0;
        if (g_shutdownSocket) {
            g_shutdownSocket->setShouldShutdown(true);
        }
    }

    void LoRaServer::acquireDbSlot() {
        std::unique_lock<std::mutex> lock(dbSemMutex_);
        dbSemCv_.wait(lock, [] { return dbSemCount_ < MAX_DB_THREADS; });
        dbSemCount_++;
    }

      void LoRaServer::releaseDbSlot() {
        {
            std::lock_guard<std::mutex> lock(dbSemMutex_);
            dbSemCount_--;
            dbSemCv_.notify_one();
        }
    }

    LoRaServer::LoRaServer(uint16_t _port, database::Database& _database, std::string _netserver_host, uint16_t _netserver_port, bool _debug)
        : port(_port), database(_database), netserver_host(common::hostToIp(_netserver_host)), netserver_port(_netserver_port), debug(_debug) {
        if (netserver_host.empty()) {
            common::handle_error_en(errno, "Failed to resolve network server hostname: " + _netserver_host);
        }
    }

    LoRaServer::~LoRaServer() = default;

    std::string makeKey(const std::string& ip, uint16_t port) {
        return ip + ":" + std::to_string(port);
    }

    void LoRaServer::init() {
        g_shutdownSocket = nullptr;

        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        auto udpServer = std::make_shared<UDPServer>();
        g_shutdownSocket = udpServer;
        UDPSocket nsSocket;

        GatewayRegistry registry(10, 5000);

        registry.setResponseCallback([&](const uint8_t* message, int length, std::string gwIp, uint16_t gwPort, GatewayEntry) {
            //if (debug) {
            //      std::cout << "SERVER response => " << message << "(" << length << ") => "
            //                << packet::LoRaPacketParser::getPacketType(message[3])
            //                << " (relay to " << gwIp << ":" << gwPort << ")" << std::endl;
            //}
            nsSocket.SendTo(message, length, gwIp, gwPort);

            if (length >= 4 && packet::LoRaPacketParser::getPacketType(message[3]) == "PKT_PULL_RESP") {
                auto pullpacket = packet::LoRaPacketParser::parse(message, length, debug);
                std::async(std::launch::async, [this, pullpacket = std::move(pullpacket)]() {
                    acquireDbSlot();
                    database::Database::insertPullDataPacketStatic(database, std::move(pullpacket));
                    releaseDbSlot();
                });
            }
        });

        udpServer.onRawMessageReceived = [&](const uint8_t* message, int length, std::string gwIp, uint16_t gwPort) {
            if (debug) {
                // std::cout << gwIp << ":" << gwPort << " => " << message << "(" << length << ") => "
                //           << (length >= 4 ? packet::LoRaPacketParser::getPacketType(message[3]) : "TOO_SHORT") << std::endl;
            }

            if (length < 12) {
                return;
            }

            std::string key = makeKey(gwIp, gwPort);

            GatewayEntry entry;
            entry.gatewayIp = gwIp;
            entry.gatewayPort = gwPort;
            entry.serverIp = netserver_host;
            entry.serverPort = netserver_port;
            entry.lastSeen = std::chrono::steady_clock::now();
            registry.registerGateway(key, entry);

            nsSocket.SendTo(message, length, netserver_host, netserver_port);

            if (packet::LoRaPacketParser::getPacketType(message[3]) == "PKT_PUSH_DATA") {
                auto pushpacket = packet::LoRaPacketParser::parse(message, length, debug);
                std::async(std::launch::async, [this, pushpacket = std::move(pushpacket)]() {
                    acquireDbSlot();
                    database::Database::insertPushDataPacketStatic(database, std::move(pushpacket));
                    releaseDbSlot();
                });
            }
        };

        std::cout << "Forwarder listening on UDP Port: " << port << std::endl;
        udpServer.Bind(port, [](int errorCode, std::string errorMessage) {
            common::handle_error_en(errorCode, errorMessage);
        });

        udpServer.run();

        g_shutdownSocket = nullptr;
        std::cout << "Shutting down gracefully..." << std::endl;
    }
}
