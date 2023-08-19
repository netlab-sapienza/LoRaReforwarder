#ifndef LORAREFORWARDER_SERVER_HPP
#define LORAREFORWARDER_SERVER_HPP

#include "database/database.hpp"

namespace lorareforwarder::server {
    class LoRaServer {
        public:
            explicit LoRaServer(uint16_t _port, database::Database& _database, std::string _netserver_host, uint16_t _netserver_port, bool _debug = false) : port {_port}, database {_database}, netserver_hostname {_netserver_host}, netserver_port {_netserver_port}, debug {_debug} { }
            ~LoRaServer();
            void init();
        private:
            uint16_t port;
            database::Database& database;
            static const int BUFFER_SIZE = 4096;

            std::string netserver_hostname, netserver_host;
            uint16_t netserver_port;

            typedef struct {
                void *socket_desc;
                size_t buffer_length;
                uint8_t *buffer;
            } threadData;

            bool debug;
    };
}

#endif /* LORAREFORWARDER_SERVER_HPP */