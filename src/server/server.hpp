#ifndef LORAREFORWARDER_SERVER_HPP
#define LORAREFORWARDER_SERVER_HPP

#include "database/database.hpp"

namespace lorareforwarder::server {
    class LoRaServer {
        public:
            explicit LoRaServer(uint16_t port, database::Database& _database);
            ~LoRaServer();

        private:
            database::Database& database;
            static const int BUFFER_SIZE = 4096;

            void init(uint16_t port);
            static void* connection_handler(void* arg);

            typedef struct {
                void *socket_desc;
                size_t buffer_length;
                uint8_t *buffer;
            } threadData;
    };
}

#endif /* LORAREFORWARDER_SERVER_HPP */