#ifndef LORAREFORWARDER_SERVER_HPP
#define LORAREFORWARDER_SERVER_HPP

#include "database/database.hpp"

namespace lorareforwarder::server {
    class LoRaServer {
        public:
            explicit LoRaServer(uint16_t _port, database::Database& _database, std::string _netserver_host, uint16_t _netserver_port) : port {_port}, database {_database}, netserver_host {_netserver_host}, netserver_port {_netserver_port} {}
            ~LoRaServer();

        private:
            uint16_t port;
            database::Database& database;
            static const int BUFFER_SIZE = 4096;

            std::string netserver_host;
            uint16_t netserver_port;

            void init();

            typedef struct {
                void *socket_desc;
                size_t buffer_length;
                uint8_t *buffer;
            } threadData;
    };
}

#endif /* LORAREFORWARDER_SERVER_HPP */