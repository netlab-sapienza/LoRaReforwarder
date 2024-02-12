#ifndef LORAREFORWARDER_PACKETPARSER_HPP
#define LORAREFORWARDER_PACKETPARSER_HPP

#include "packet.hpp"
#include <string>

namespace lorareforwarder::packet {
    class LoRaPacketParser {
        public:
            explicit LoRaPacketParser() = default;
            ~LoRaPacketParser() = default;

            static LoRaPacket* parse(const uint8_t *packet_buf, size_t length, bool debug = false);
            static std::string getPacketType(const uint8_t buf);

        private:
            static void parsePushJson(const char *string, LoRaPacket* packet, bool debug = false);
            static void parsePullJson(const char *string, LoRaPacket* packet, bool debug = false);

    };
}

#endif /* LORAREFORWARDER_PACKETPARSER_HPP */