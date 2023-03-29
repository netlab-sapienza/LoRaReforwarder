#include <stdio.h>
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "packetParser.hpp"

namespace lorareforwarder::packet {
    std::string LoRaPacketParser::getPacketType(const uint8_t buf) {

        std::string type;

        switch(buf) {
            case PKT_PUSH_DATA:
                type = "PKT_PUSH_DATA";
                break;
            case PKT_PUSH_ACK:
                type = "PKT_PUSH_ACK";
                break;
            case PKT_PULL_DATA:
                type = "PKT_PULL_DATA";
                break;
            case PKT_PULL_RESP:
                type = "PKT_PULL_RESP";
                break;
            case PKT_PULL_ACK:
                type = "PKT_PULL_ACK";
                break;
            case PKT_TX_ACK:
                type = "PKT_TX_ACK";
                break;
            default:
                type = "UNDEFINED";
        };

        return type;
    }

    LoRaPacket* LoRaPacketParser::parse(const uint8_t *packet_buf, size_t length) {

        LoRaPacket* packet = (LoRaPacket*) malloc(sizeof(LoRaPacket));
        packet->buf = packet_buf;
        packet->length = length;

        /* check and parse the payload */
        if (length < 12) { /* not enough bytes for packet from gateway */
            printf(" (too short for GW <-> MAC protocol)\n");

            packet->isValid = false;
            return packet;
        }

        if (packet_buf[0] != PKT_PROTOCOL_VERSION) { /* check protocol version number */
            printf(", invalid version %u\n", packet_buf[0]);

            packet->isValid = false;
            return packet;
        }

        packet->raw_mac_h = *((uint32_t *)(packet_buf + 4));
        packet->raw_mac_l = *((uint32_t *)(packet_buf + 8));
        packet->gw_mac = ((uint64_t) ntohl(packet->raw_mac_h) << 32) + (uint64_t) ntohl(packet->raw_mac_l);

        /* interpret gateway command */

        if(packet_buf[3] == PKT_PUSH_DATA) {
            printf("PUSH_DATA from gateway 0x%08X%08X\n", (uint32_t)(packet->gw_mac >> 32), (uint32_t)(packet->gw_mac & 0xFFFFFFFF));
            packet->ack_command = PKT_PUSH_ACK;
            printf("<-  pkt out, PUSH_ACK for host");            

            char* string = (char*) (packet_buf + 12);
            printf("La stringa che contiene e' %s\n", string);
            parseJson(string, packet);
        } else if(packet_buf[3] == PKT_PULL_DATA) {
            printf(", PULL_DATA from gateway 0x%08X%08X\n", (uint32_t)(packet->gw_mac >> 32), (uint32_t)(packet->gw_mac & 0xFFFFFFFF));
            packet->ack_command = PKT_PULL_ACK;
            printf("<-  pkt out, PULL_ACK for host");
        } else {
            printf(", unexpected command %u\n", packet_buf[3]);
            packet->isValid = false;
        }          
    
        return packet;
    }

    void LoRaPacketParser::parseJson(const char *string, LoRaPacket* packet) {
        json packjson = json::parse(string);
        std::cout << "rxpk: " << packjson["rxpk"] << std::endl;
        std::cout << "stat: " << packjson["stat"] << std::endl;

        if(!packjson["rxpk"].is_null()) {
            packet->rxpk_buf = (rxpk*) malloc(sizeof(rxpk));

            packet->rxpk_buf->time = packjson["rxpk"][0]["time"];
            packet->rxpk_buf->tmms = packjson["rxpk"][0]["tmms"];
            packet->rxpk_buf->tmst = packjson["rxpk"][0]["tmst"];
            packet->rxpk_buf->chan = packjson["rxpk"][0]["chan"];
            packet->rxpk_buf->rfch = packjson["rxpk"][0]["rfch"];
            packet->rxpk_buf->stat = packjson["rxpk"][0]["stat"];
            packet->rxpk_buf->freq = packjson["rxpk"][0]["freq"];
            packet->rxpk_buf->datr = packjson["rxpk"][0]["brd"]; // DA VERIFICARE
            packet->rxpk_buf->rssi = packjson["rxpk"][0]["rssi"];
            packet->rxpk_buf->datrid = packjson["rxpk"][0]["datr"];
            packet->rxpk_buf->modu = packjson["rxpk"][0]["modu"]; // DA VEFIRIFICARE SU DB
            packet->rxpk_buf->codr = packjson["rxpk"][0]["codr"];
            packet->rxpk_buf->lsnr = packjson["rxpk"][0]["lsnr"];
            packet->rxpk_buf->size = packjson["rxpk"][0]["size"];
            packet->rxpk_buf->data = packjson["rxpk"][0]["data"];
        }

        if(!packjson["stat"].is_null()) {
            packet->stat_buf = (stat*) malloc(sizeof(stat));

            packet->stat_buf->time = packjson["stat"]["time"];
            packet->stat_buf->lati = packjson["stat"]["lati"];
            packet->stat_buf->longi = packjson["stat"]["long"];
            packet->stat_buf->alti = packjson["stat"]["alti"];
            packet->stat_buf->rxnb = packjson["stat"]["rxnb"];
            packet->stat_buf->rxok = packjson["stat"]["rxok"];
            packet->stat_buf->rxfw = packjson["stat"]["rxfw"];
            packet->stat_buf->ackr = packjson["stat"]["ackr"];
            packet->stat_buf->dwnb = packjson["stat"]["dwnb"];
            packet->stat_buf->txnb = packjson["stat"]["txnb"];
        }
    }
}