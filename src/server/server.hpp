#ifndef LORAREFORWARDER_SERVER_HPP
#define LORAREFORWARDER_SERVER_HPP

#include "../database/database.hpp"
#include <mutex>
#include <condition_variable>
#include <memory>

namespace lorareforwarder::server {
    class LoRaServer {
        public:
            explicit LoRaServer(uint16_t _port, database::Database& _database, std::string _netserver_host, uint16_t _netserver_port, bool _debug = false);
            ~LoRaServer();
            void init();

        private:
            static constexpr int MAX_DB_THREADS = 16;
            inline static std::mutex dbSemMutex_;
            inline static std::condition_variable dbSemCv_;
            inline static int dbSemCount_ = 0;
            static void acquireDbSlot();
            static void releaseDbSlot();
            uint16_t port;
            database::Database& database;
            std::string netserver_host;
            uint16_t netserver_port;
            bool debug;
    };
}

#endif /* LORAREFORWARDER_SERVER_HPP */
