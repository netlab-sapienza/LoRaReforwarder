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

        switch(packet_buf[3]) {
            case PKT_PUSH_DATA: {
                packet->type = PKT_PUSH_DATA;
                printf("PUSH_DATA from gateway 0x%08X%08X\n", (uint32_t)(packet->gw_mac >> 32), (uint32_t)(packet->gw_mac & 0xFFFFFFFF));
                packet->ack_command = PKT_PUSH_ACK;

                char* string = (char*) (packet_buf + 12);
                printf("La stringa che contiene e' %s\n", string);
                parsePushJson(string, packet);
                break;
            }
            case PKT_PUSH_ACK: {
                packet->type = PKT_PUSH_ACK;
                break;
            }
            case PKT_PULL_DATA: {
                packet->type = PKT_PULL_DATA;

                printf(", PULL_DATA from gateway 0x%08X%08X\n", (uint32_t)(packet->gw_mac >> 32), (uint32_t)(packet->gw_mac & 0xFFFFFFFF));
                packet->ack_command = PKT_PULL_ACK;
                printf("<-  pkt out, PULL_ACK for host");
                break;
            }
            case PKT_PULL_RESP: {
                packet->type = PKT_PULL_RESP;

                char* string = (char*) (packet_buf + 4);
                printf("La stringa che contiene e' %s\n", string);
                parsePullJson(string, packet);
                break;
            }
            case PKT_PULL_ACK: {
                packet->type = PKT_PULL_ACK;

                break;
            }
            case PKT_TX_ACK: {
                packet->type = PKT_TX_ACK;

                break;
            }
            default: {
                printf(", unexpected command %u\n", packet_buf[3]);
                packet->isValid = false;
            }
        };  
    
        return packet;
    }

    void LoRaPacketParser::parsePushJson(const char *string, LoRaPacket* packet) {
        json packjson = json::parse(string);
        std::cout << "rxpk: " << packjson["rxpk"] << std::endl;
        std::cout << "stat: " << packjson["stat"] << std::endl;

        if(!packjson["rxpk"].is_null() && packjson["rxpk"].is_array()) {
            packet->rxpk_buf = (rxpk**) malloc(packjson["rxpk"].size() * sizeof(rxpk*));
            for(int i = 0; i < packjson["rxpk"].size(); i++) {
                packet->rxpk_buf[i] = (rxpk*) malloc(sizeof(rxpk));

                packet->rxpk_buf[i]->time = packjson["rxpk"][i]["time"];
                packet->rxpk_buf[i]->tmms = packjson["rxpk"][i]["tmms"];
                packet->rxpk_buf[i]->tmst = packjson["rxpk"][i]["tmst"];
                packet->rxpk_buf[i]->chan = packjson["rxpk"][i]["chan"];
                packet->rxpk_buf[i]->rfch = packjson["rxpk"][i]["rfch"];
                packet->rxpk_buf[i]->stat = packjson["rxpk"][i]["stat"];
                packet->rxpk_buf[i]->freq = packjson["rxpk"][i]["freq"];
                packet->rxpk_buf[i]->datr = packjson["rxpk"][i]["brd"]; // TODO Protocol documentation check
                packet->rxpk_buf[i]->rssi = packjson["rxpk"][i]["rssi"];
                packet->rxpk_buf[i]->datrid = packjson["rxpk"][i]["datr"];
                packet->rxpk_buf[i]->modu = packjson["rxpk"][i]["modu"]; // TODO DB check
                packet->rxpk_buf[i]->codr = packjson["rxpk"][i]["codr"];
                packet->rxpk_buf[i]->lsnr = packjson["rxpk"][i]["lsnr"];
                packet->rxpk_buf[i]->size = packjson["rxpk"][i]["size"];
                packet->rxpk_buf[i]->data = packjson["rxpk"][i]["data"];
            }
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

    void LoRaPacketParser::parsePullJson(const char *string, LoRaPacket* packet) {
        json packjson = json::parse(string);
        std::cout << "txpk: " << packjson["txpk"] << std::endl;

        if(!packjson["txpk"].is_null()) {
            packet->txpk_buf = (txpk*) malloc(sizeof(txpk));

            if(packjson["txpk"]["imme"].is_null())
                packet->txpk_buf->imme = false;
            else
                packet->txpk_buf->imme = packjson["txpk"]["imme"];

            if(packjson["txpk"]["tmst"].is_null())
                packet->txpk_buf->tmst = mariadb::value::null;
            else
                packet->txpk_buf->tmst = packjson["txpk"]["tmst"];

            if(packjson["txpk"]["tmms"].is_null())
                packet->txpk_buf->tmms = mariadb::value::null;
            else
                packet->txpk_buf->tmms = packjson["txpk"]["tmms"];

            if(packjson["txpk"]["freq"].is_null())
                packet->txpk_buf->freq = mariadb::value::null;
            else
                packet->txpk_buf->freq = packjson["txpk"]["freq"];

            if(packjson["txpk"]["rfch"].is_null())
                packet->txpk_buf->rfch = mariadb::value::null;
            else
                packet->txpk_buf->rfch = packjson["txpk"]["rfch"];

            if(packjson["txpk"]["powe"].is_null())
                packet->txpk_buf->powe = mariadb::value::null;
            else
                packet->txpk_buf->powe = packjson["txpk"]["powe"];
            
            packet->txpk_buf->modu = packjson["txpk"]["modu"].is_null() ? "" : packjson["txpk"]["modu"];

            packet->txpk_buf->datrid = packjson["txpk"]["datrid"].is_null() ? "" : packjson["txpk"]["datrid"];

            if(packjson["txpk"]["datr"].is_null()) // TODO Protocol documentation check
                packet->txpk_buf->datr = mariadb::value::null;
            else
                packet->txpk_buf->data = packjson["txpk"]["datr"];
            
            packet->txpk_buf->codr = packjson["txpk"]["codr"].is_null() ? "" : packjson["txpk"]["codr"];

            if(packjson["txpk"]["fdev"].is_null())
                packet->txpk_buf->fdev = mariadb::value::null;
            else
                packet->txpk_buf->fdev = packjson["txpk"]["fdev"];

            if(packjson["txpk"]["ipol"].is_null())
                packet->txpk_buf->ipol = mariadb::value::null;
            else
                packet->txpk_buf->ipol = packjson["txpk"]["ipol"];

            if(packjson["txpk"]["prea"].is_null())
                packet->txpk_buf->prea = mariadb::value::null;
            else
                packet->txpk_buf->prea = packjson["txpk"]["prea"];

            if(packjson["txpk"]["size"].is_null())
                packet->txpk_buf->size = mariadb::value::null;
            else
                packet->txpk_buf->size = packjson["txpk"]["size"];

            packet->txpk_buf->data = packjson["txpk"]["data"].is_null() ? "" : packjson["txpk"]["data"];

            if(packjson["txpk"]["ncrc"].is_null())
                packet->txpk_buf->ncrc = false;
            else
                packet->txpk_buf->ncrc = packjson["txpk"]["ncrc"];
        }
    }
}