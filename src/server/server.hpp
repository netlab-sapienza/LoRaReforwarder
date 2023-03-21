#ifndef LORAREFORWARDER_SERVER_HPP
#define LORAREFORWARDER_SERVER_HPP

namespace lorareforwarder::server {
    class LoRaServer {
        public:
            explicit LoRaServer(uint16_t port);
            ~LoRaServer();

        private:
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