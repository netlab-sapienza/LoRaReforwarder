#ifndef LORAREFORWARDER_DATABASE_HPP
#define LORAREFORWARDER_DATABASE_HPP

#include <mariadb/conncpp.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <mutex>
#include "packet/packet.hpp"

namespace lorareforwarder::database {
    using ::lorareforwarder::packet;
    class Database {
        public:
            explicit Database(const std::string &host_name, const std::string &user_name, const std::string &password,
                              const std::string &schema, uint32_t port, bool _debug);
            ~Database();

            bool isConnected() { return connected; }

            void insertPushDataPacket(std::unique_ptr<packet::LoRaPacket> packet);
            void insertPullDataPacket(std::unique_ptr<packet::LoRaPacket> packet);

            static std::unique_ptr<sql::Connection> generateConnection(sql::Driver* driver, const sql::SQLString &url, sql::Properties &props);

            static void insertPushDataPacketStatic(Database& db, std::unique_ptr<packet::LoRaPacket> packet) {
                db.insertPushDataPacket(std::move(packet));
            }
            static void insertPullDataPacketStatic(Database& db, std::unique_ptr<packet::LoRaPacket> packet) {
                db.insertPullDataPacket(std::move(packet));
            }
        private:
            std::unique_ptr<sql::Connection> conn;
            bool connected = false;
            bool debug;
            mutable std::mutex dbMutex_;
    };
}

#endif /* LORAREFORWARDER_DATABASE_HPP */
