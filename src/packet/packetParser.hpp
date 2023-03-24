#ifndef LORAREFORWARDER_PACKETPARSER_HPP
#define LORAREFORWARDER_PACKETPARSER_HPP

#define PROTOCOL_VERSION 2

#define PKT_PUSH_DATA    0
#define PKT_PUSH_ACK     1
#define PKT_PULL_DATA    2
#define PKT_PULL_RESP    3
#define PKT_PULL_ACK     4

#include "packet.hpp"
#include <string>

namespace lorareforwarder::packet {
    class LoRaPacketParser {
        public:
            explicit LoRaPacketParser() = default;
            ~LoRaPacketParser() = default;

            static LoRaPacket parse(const uint8_t *packet_buf, size_t length);
            static std::string getPacketType(const uint8_t buf);
        private:
            
    };
}

#endif /* LORAREFORWARDER_PACKETPARSER_HPP */