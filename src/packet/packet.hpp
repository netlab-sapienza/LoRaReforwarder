#ifndef LORAREFORWARDER_PACKET_HPP
#define LORAREFORWARDER_PACKET_HPP

#define PROTOCOL_VERSION 2

#define PKT_PUSH_DATA    0
#define PKT_PUSH_ACK     1
#define PKT_PULL_DATA    2
#define PKT_PULL_RESP    3
#define PKT_PULL_ACK     4

#include <stdlib.h>

namespace lorareforwarder::packet {
    class LoRaPacket {
        public:
            LoRaPacket() = default;
            ~LoRaPacket() = default;

            const uint8_t *buf;
            size_t length;

            uint32_t raw_mac_h; /* Most Significant Nibble, network order */
            uint32_t raw_mac_l; /* Least Significant Nibble, network order */
            uint64_t gw_mac; /* MAC address of the client (gateway) */

            uint8_t ack_command;

            bool isValid = true;
    };
}

#endif /* LORAREFORWARDER_PACKET_HPP */