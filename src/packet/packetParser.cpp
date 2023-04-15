#include "common/common.hpp"
#include <stdio.h>
#include <iostream>

#include <arpa/inet.h>
#include <nlohmann/json.hpp>
#include <cstring>
using json = nlohmann::json;

#include <mariadb/conncpp/Types.hpp>
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
        LoRaPacket *packet = new LoRaPacket;
        if(!packet) common::handle_error("Can't allocate memory for LoRaPacket instance");

        packet->buf = packet_buf;
        packet->length = length;

        /* check and parse the payload */
        if (length < 12) { /* not enough bytes for packet from gateway */
            packet->isValid = false;
            return packet;
        }

        if (packet_buf[0] != PKT_PROTOCOL_VERSION) { /* check protocol version number */
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

                //printf("PULL_DATA from gateway 0x%08X%08X\n", (uint32_t)(packet->gw_mac >> 32), (uint32_t)(packet->gw_mac & 0xFFFFFFFF));
                packet->ack_command = PKT_PULL_ACK;
                break;
            }
            case PKT_PULL_RESP: {
                packet->type = PKT_PULL_RESP;

                char* string = (char*) (packet_buf + 4);
                //printf("La stringa che contiene e' %s\n", string);
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
                printf("Unexpected command %u\n", packet_buf[3]);
                packet->isValid = false;
            }
        };  
    
        return packet;
    }

    void LoRaPacketParser::parsePushJson(const char *string, LoRaPacket* packet) {
        json packjson = json::parse(string);

        if(!packjson["rxpk"].is_null() && packjson["rxpk"].is_array()) {
            try {
                packet->rxpk_buf.reserve(packjson["rxpk"].size());

                for(int i = 0; i < packjson["rxpk"].size(); i++) {
                    rxpk actualrxpk;
                    packet->rxpk_buf.push_back(actualrxpk);

                    if(!packjson["rxpk"][i]["time"].is_null()) {
                        std::string time = packjson["rxpk"][i]["time"];
                        packet->rxpk_buf[i].time = time;
                    } else
                        packet->rxpk_buf[i].time = sql::_NULL;

                    if(!packjson["rxpk"][i]["tmms"].is_null())
                        packet->rxpk_buf[i].tmms = packjson["rxpk"][i]["tmms"];
                    else
                        packet->rxpk_buf[i].tmms = sql::_NULL;

                    if(!packjson["rxpk"][i]["tmst"].is_null())
                        packet->rxpk_buf[i].tmst = packjson["rxpk"][i]["tmst"];
                    else
                        packet->rxpk_buf[i].tmst = sql::_NULL;

                    if(!packjson["rxpk"][i]["chan"].is_null())
                        packet->rxpk_buf[i].chan = packjson["rxpk"][i]["chan"];
                    else
                        packet->rxpk_buf[i].chan = sql::_NULL;

                    if(!packjson["rxpk"][i]["rfch"].is_null())
                        packet->rxpk_buf[i].rfch = packjson["rxpk"][i]["rfch"];
                    else
                        packet->rxpk_buf[i].rfch = sql::_NULL;

                    if(!packjson["rxpk"][i]["stat"].is_null())
                        packet->rxpk_buf[i].stat = packjson["rxpk"][i]["stat"];
                    else
                        packet->rxpk_buf[i].stat = sql::_NULL;

                    if(!packjson["rxpk"][i]["freq"].is_null())
                        packet->rxpk_buf[i].freq = packjson["rxpk"][i]["freq"];
                    else
                        packet->rxpk_buf[i].freq = sql::_NULL;

                    if(!packjson["rxpk"][i]["rssi"].is_null())
                        packet->rxpk_buf[i].rssi = packjson["rxpk"][i]["rssi"];
                    else
                        packet->rxpk_buf[i].rssi = sql::_NULL;

                    if(!packjson["rxpk"][i]["datr"].is_null()) {
                        std::string datr = packjson["rxpk"][i]["datr"];
                        packet->rxpk_buf[i].datr = datr;
                    }
                    else
                        packet->rxpk_buf[i].datr = sql::_NULL;

                    if(!packjson["rxpk"][i]["modu"].is_null()) { // TODO DB check
                        std::string modu = packjson["rxpk"][i]["modu"];
                        packet->rxpk_buf[i].modu = modu;
                    }
                    else
                        packet->rxpk_buf[i].modu = sql::_NULL;

                    if(!packjson["rxpk"][i]["codr"].is_null()) {
                        std::string codr = packjson["rxpk"][i]["codr"];
                        packet->rxpk_buf[i].codr = codr;
                    }
                    else
                        packet->rxpk_buf[i].codr = sql::_NULL;

                    if(!packjson["rxpk"][i]["lsnr"].is_null())
                        packet->rxpk_buf[i].lsnr = packjson["rxpk"][i]["lsnr"];
                    else
                        packet->rxpk_buf[i].lsnr = sql::_NULL;

                    if(!packjson["rxpk"][i]["size"].is_null())
                        packet->rxpk_buf[i].size = packjson["rxpk"][i]["size"];
                    else
                        packet->rxpk_buf[i].size = sql::_NULL;

                    if(!packjson["rxpk"][i]["data"].is_null()) {
                        std::string data = packjson["rxpk"][i]["data"];
                        packet->rxpk_buf[i].data = packjson["rxpk"][i]["data"];
                    }
                    else
                        packet->rxpk_buf[i].data = sql::_NULL;

                }

            } catch (std::exception const& e) {
                common::handle_error("Can't allocate memory for rxpk instance");
                std::cerr << "Exception: " << e.what() << std::endl;
                return;
            }
        }

        if(!packjson["stat"].is_null()) {
            std::cout << "stat: " << packjson["stat"] << std::endl;

            packet->stat_buf = new stat;
            if(!packet->stat_buf) common::handle_error("Can't allocate memory for stat instance");

            if(!packjson["stat"]["time"].is_null())
                packet->stat_buf->time = packjson["stat"]["time"];
            else
                packet->stat_buf->time = sql::_NULL;

            if(!packjson["stat"]["lati"].is_null())
                packet->stat_buf->lati = packjson["stat"]["lati"];
            else
                packet->stat_buf->lati = sql::_NULL;

            if(!packjson["stat"]["long"].is_null())
                packet->stat_buf->longi = packjson["stat"]["long"];
            else
                packet->stat_buf->longi = sql::_NULL;

            if(!packjson["stat"]["alti"].is_null())
                packet->stat_buf->alti = packjson["stat"]["alti"];
            else
                packet->stat_buf->alti = sql::_NULL;

            if(!packjson["stat"]["rxnb"].is_null())
                packet->stat_buf->rxnb = packjson["stat"]["rxnb"];
            else
                packet->stat_buf->rxnb = sql::_NULL;

            if(!packjson["stat"]["rxok"].is_null())
                packet->stat_buf->rxok = packjson["stat"]["rxok"];
            else
                packet->stat_buf->rxok = sql::_NULL;

            if(!packjson["stat"]["rxfw"].is_null())
                packet->stat_buf->rxfw = packjson["stat"]["rxfw"];
            else
                packet->stat_buf->rxfw = sql::_NULL;

            if(!packjson["stat"]["ackr"].is_null())
                packet->stat_buf->ackr = packjson["stat"]["ackr"];
            else
                packet->stat_buf->ackr = sql::_NULL;

            if(!packjson["stat"]["dwnb"].is_null())
                packet->stat_buf->dwnb = packjson["stat"]["dwnb"];
            else
                packet->stat_buf->dwnb = sql::_NULL;

            if(!packjson["stat"]["txnb"].is_null())
                packet->stat_buf->txnb = packjson["stat"]["txnb"];
            else
                packet->stat_buf->txnb = sql::_NULL;
        }
    }

    void LoRaPacketParser::parsePullJson(const char *string, LoRaPacket* packet) {
        json packjson = json::parse(string);

        if(!packjson["txpk"].is_null()) {
            std::cout << "txpk: " << packjson["txpk"] << std::endl;

            packet->txpk_buf = new txpk;
            if(!packet->txpk_buf) common::handle_error("Can't allocate memory for txpk instance");

            if(packjson["txpk"]["imme"].is_null())
                packet->txpk_buf->imme = false;
            else
                packet->txpk_buf->imme = packjson["txpk"]["imme"];

            if(packjson["txpk"]["tmst"].is_null())
                packet->txpk_buf->tmst = sql::_NULL;
            else
                packet->txpk_buf->tmst = packjson["txpk"]["tmst"];

            if(packjson["txpk"]["tmms"].is_null())
                packet->txpk_buf->tmms = sql::_NULL;
            else
                packet->txpk_buf->tmms = packjson["txpk"]["tmms"];

            if(packjson["txpk"]["freq"].is_null())
                packet->txpk_buf->freq = sql::_NULL;
            else
                packet->txpk_buf->freq = packjson["txpk"]["freq"];

            if(packjson["txpk"]["rfch"].is_null())
                packet->txpk_buf->rfch = sql::_NULL;
            else
                packet->txpk_buf->rfch = packjson["txpk"]["rfch"];

            if(packjson["txpk"]["powe"].is_null())
                packet->txpk_buf->powe = sql::_NULL;
            else
                packet->txpk_buf->powe = packjson["txpk"]["powe"];

            if(packjson["txpk"]["modu"].is_null())
                packet->txpk_buf->modu = sql::_NULL;
            else
                packet->txpk_buf->modu = packjson["txpk"]["modu"];

            if(packjson["txpk"]["datr"].is_null())
                packet->txpk_buf->datr = sql::_NULL;
            else
                packet->txpk_buf->datr = packjson["txpk"]["datr"];

            if(packjson["txpk"]["codr"].is_null())
                packet->txpk_buf->codr = sql::_NULL;
            else
                packet->txpk_buf->codr = packjson["txpk"]["codr"];
            
            if(packjson["txpk"]["fdev"].is_null())
                packet->txpk_buf->fdev = sql::_NULL;
            else
                packet->txpk_buf->fdev = packjson["txpk"]["fdev"];

            if(packjson["txpk"]["ipol"].is_null())
                packet->txpk_buf->ipol = sql::_NULL;
            else
                packet->txpk_buf->ipol = packjson["txpk"]["ipol"];

            if(packjson["txpk"]["prea"].is_null())
                packet->txpk_buf->prea = sql::_NULL;
            else
                packet->txpk_buf->prea = packjson["txpk"]["prea"];

            if(packjson["txpk"]["size"].is_null())
                packet->txpk_buf->size = sql::_NULL;
            else
                packet->txpk_buf->size = packjson["txpk"]["size"];

            if(packjson["txpk"]["data"].is_null())
                packet->txpk_buf->data = sql::_NULL;
            else
                packet->txpk_buf->data = packjson["txpk"]["data"];

            if(packjson["txpk"]["ncrc"].is_null())
                packet->txpk_buf->ncrc = false;
            else
                packet->txpk_buf->ncrc = packjson["txpk"]["ncrc"];
        }
    }
}