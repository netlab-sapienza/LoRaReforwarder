#ifndef LORAREFORWARDER_DATABASE_HPP
#define LORAREFORWARDER_DATABASE_HPP

#include <mariadb/conncpp.hpp>

#include "packet/packet.hpp"

namespace lorareforwarder::database {
    class Database {
        public:
            explicit Database(const std::string &host_name, const std::string &user_name, const std::string &password,
                              const std::string &schema, uint32_t port);
            ~Database();
            
            int insertPushDataPacket(packet::LoRaPacket* packet);
            int insertPullDataPacket(packet::LoRaPacket* packet);

            static std::unique_ptr<sql::Connection> generateConnection(sql::Driver* driver, const sql::SQLString &url, sql::Properties &props);

            static int insertPushDataPacketStatic(Database& db, packet::LoRaPacket* packet) {
                return db.insertPushDataPacket(packet);
            }
            static int insertPullDataPacketStatic(Database& db, packet::LoRaPacket* packet) {
                return db.insertPullDataPacket(packet);
            }
        private:
            std::unique_ptr<sql::Connection> conn;
    };
}

#endif /* LORAREFORWARDER_DATABASE_HPP */