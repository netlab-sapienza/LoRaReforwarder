#ifndef LORAREFORWARDER_DATABASE_HPP
#define LORAREFORWARDER_DATABASE_HPP

#include <mariadb++/types.hpp>
#include <mariadb++/account.hpp>
#include <mariadb++/connection.hpp>

#include "packet/packet.hpp"

namespace lorareforwarder::database {
    class Database {
        public:
            explicit Database(const std::string &host_name, const std::string &user_name, const std::string &password,
                              const std::string &schema, mariadb::u32 port);
            ~Database();
            
            int insertPushDataPacket(packet::LoRaPacket* packet);
            int insertPullDataPacket(packet::LoRaPacket* packet);

            static int insertPushDataPacketStatic(Database& db, packet::LoRaPacket* packet) {
                return db.insertPushDataPacket(packet);
            }
            static int insertPullDataPacketStatic(Database& db, packet::LoRaPacket* packet) {
                return db.insertPullDataPacket(packet);
            }
        private:
            mariadb::account_ref acc;
            mariadb::connection_ref con;
    };
}

#endif /* LORAREFORWARDER_DATABASE_HPP */