#include <string.h>
#include <stdio.h>

#include "packetParser.hpp"

namespace lorareforwarder::packet {
    LoRaPacket LoRaPacketParser::parse(const uint8_t *packet_buf, size_t length) {

        LoRaPacket packet;
        packet.buf = packet_buf;
        packet.length = length;

        /* check and parse the payload */
        if (length < 12) { /* not enough bytes for packet from gateway */
            printf(" (too short for GW <-> MAC protocol)\n");

            packet.isValid = false;
            return packet;
        }

        if (packet_buf[0] != PROTOCOL_VERSION) { /* check protocol version number */
            printf(", invalid version %u\n", packet_buf[0]);

            packet.isValid = false;
            return packet;
        }

        packet.raw_mac_h = *((uint32_t *)(packet_buf + 4));
        packet.raw_mac_l = *((uint32_t *)(packet_buf + 8));
        packet.gw_mac = ((uint64_t)ntohl(packet.raw_mac_h) << 32) + (uint64_t)ntohl(packet.raw_mac_l);

        /* interpret gateway command */
        switch (packet_buf[3]) {
            case PKT_PUSH_DATA:
                printf(", PUSH_DATA from gateway 0x%08X%08X\n", (uint32_t)(packet.gw_mac >> 32), (uint32_t)(packet.gw_mac & 0xFFFFFFFF));
                packet.ack_command = PKT_PUSH_ACK;
                printf("<-  pkt out, PUSH_ACK for host");            

                char* string = (char*) (packet_buf + 12);
                printf("La stringa che contiene e' %s\n", string);
                break;
            case PKT_PULL_DATA:
                printf(", PULL_DATA from gateway 0x%08X%08X\n", (uint32_t)(packet.gw_mac >> 32), (uint32_t)(packet.gw_mac & 0xFFFFFFFF));
                packet.ack_command = PKT_PULL_ACK;
                printf("<-  pkt out, PULL_ACK for host");
                break;
            default:
                printf(", unexpected command %u\n", packet_buf[3]);
                packet.isValid = false;
                return packet;
        }          
    
        return packet;
    }
}