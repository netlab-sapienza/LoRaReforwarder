#ifndef LORAREFORWARDER_PACKETPARSER_HPP
#define LORAREFORWARDER_PACKETPARSER_HPP

#define PROTOCOL_VERSION 2

#define PKT_PUSH_DATA    0
#define PKT_PUSH_ACK     1
#define PKT_PULL_DATA    2
#define PKT_PULL_RESP    3
#define PKT_PULL_ACK     4

#include "packet.hpp"

namespace lorareforwarder::packet {
    class LoRaPacketParser {
        public:
            LoRaPacketParser() = default;
            ~LoRaPacketParser() = default;

            static LoRaPacket parse(const uint8_t *packet_buf, size_t length);
        private:
            
    };
}

#endif /* LORAREFORWARDER_PACKETPARSER_HPP */