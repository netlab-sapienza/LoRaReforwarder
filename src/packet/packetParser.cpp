#include "common/common.hpp"
#include <stdio.h>
#include <limits>
#include <iostream>

#include <arpa/inet.h>
#include <nlohmann/json.hpp>
#include <cstring>
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

    std::unique_ptr<LoRaPacket> LoRaPacketParser::parse(const uint8_t *packet_buf, size_t length, bool debug) {
        auto packet = std::make_unique<LoRaPacket>();

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

        {
            uint32_t raw_h, raw_l;
            memcpy(&raw_h, packet_buf + 4, sizeof(raw_h));
            memcpy(&raw_l, packet_buf + 8, sizeof(raw_l));
            packet->raw_mac_h = ntohl(raw_h);
            packet->raw_mac_l = ntohl(raw_l);
            packet->gw_mac = ((uint64_t)packet->raw_mac_h << 32) | packet->raw_mac_l;
        }

        /* interpret gateway command */
        switch(packet_buf[3]) {
            case PKT_PUSH_DATA: {
                packet->type = PKT_PUSH_DATA;
                if(debug) printf("PUSH_DATA from gateway 0x%08X%08X\n", (uint32_t)(packet->gw_mac >> 32), (uint32_t)(packet->gw_mac & 0xFFFFFFFF));
                packet->ack_command = PKT_PUSH_ACK;

                char* string = (char*) (packet_buf + 12);
                parsePushJson(string, *packet, debug);
                break;
            }
            case PKT_PUSH_ACK: {
                packet->type = PKT_PUSH_ACK;
                break;
            }
            case PKT_PULL_DATA: {
                packet->type = PKT_PULL_DATA;

                if(debug) printf("PULL_DATA from gateway 0x%08X%08X\n", (uint32_t)(packet->gw_mac >> 32), (uint32_t)(packet->gw_mac & 0xFFFFFFFF));
                packet->ack_command = PKT_PULL_ACK;
                break;
            }
            case PKT_PULL_RESP: {
                packet->type = PKT_PULL_RESP;

                char* string = (char*) (packet_buf + 4);
                parsePullJson(string, *packet, debug);
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
                if(debug) printf("Invalid packet %u\n", packet_buf[3]);
                packet->isValid = false;
            }
        };  
    
        return packet;
    }

    void LoRaPacketParser::parsePushJson(const char *string, LoRaPacket& packet, bool debug) {
        json packjson;
        try {
            packjson = json::parse(string);
        } catch (const std::exception& e) {
            if (debug) std::cerr << "JSON parse error in push data: " << e.what() << std::endl;
            return;
        }

        if(!packjson["rxpk"].is_null() && packjson["rxpk"].is_array()) {
            try {
                packet.rxpk_buf.reserve(packjson["rxpk"].size());

                for(int i = 0; i < packjson["rxpk"].size(); i++) {
                    rxpk actualrxpk;
                    packet.rxpk_buf.push_back(actualrxpk);

                    if(!packjson["rxpk"][i]["time"].is_null()) {
                        std::string time = packjson["rxpk"][i]["time"];
                        packet.rxpk_buf[i].time = time;
                    } else
                        packet.rxpk_buf[i].time = "";

                    if(!packjson["rxpk"][i]["tmms"].is_null())
                        packet.rxpk_buf[i].tmms = packjson["rxpk"][i]["tmms"];
                    else
                        packet.rxpk_buf[i].tmms = std::numeric_limits<uint64_t>::max();

                    if(!packjson["rxpk"][i]["tmst"].is_null())
                        packet.rxpk_buf[i].tmst = packjson["rxpk"][i]["tmst"];
                    else
                        packet.rxpk_buf[i].tmst = std::numeric_limits<uint32_t>::max();

                    if(!packjson["rxpk"][i]["chan"].is_null())
                        packet.rxpk_buf[i].chan = packjson["rxpk"][i]["chan"];
                    else
                        packet.rxpk_buf[i].chan = std::numeric_limits<uint32_t>::max();

                    if(!packjson["rxpk"][i]["rfch"].is_null())
                        packet.rxpk_buf[i].rfch = packjson["rxpk"][i]["rfch"];
                    else
                        packet.rxpk_buf[i].rfch = std::numeric_limits<uint32_t>::max();

                    if(!packjson["rxpk"][i]["stat"].is_null())
                        packet.rxpk_buf[i].stat = packjson["rxpk"][i]["stat"];
                    else
                        packet.rxpk_buf[i].stat = std::numeric_limits<uint32_t>::max();

                    if(!packjson["rxpk"][i]["freq"].is_null())
                        packet.rxpk_buf[i].freq = packjson["rxpk"][i]["freq"];
                    else
                        packet.rxpk_buf[i].freq = std::numeric_limits<float>::max();

                    if(!packjson["rxpk"][i]["rssi"].is_null())
                        packet.rxpk_buf[i].rssi = packjson["rxpk"][i]["rssi"];
                    else
                        packet.rxpk_buf[i].rssi = std::numeric_limits<int32_t>::max();

                    if(!packjson["rxpk"][i]["datr"].is_null()) {
                        std::string datr = packjson["rxpk"][i]["datr"];
                        packet.rxpk_buf[i].datr = datr;
                    }
                    else
                        packet.rxpk_buf[i].datr = "";

                    if(!packjson["rxpk"][i]["modu"].is_null()) {
                        std::string modu = packjson["rxpk"][i]["modu"];
                        packet.rxpk_buf[i].modu = modu;
                    }
                    else
                        packet.rxpk_buf[i].modu = "";

                    if(!packjson["rxpk"][i]["codr"].is_null()) {
                        std::string codr = packjson["rxpk"][i]["codr"];
                        packet.rxpk_buf[i].codr = codr;
                    }
                    else
                        packet.rxpk_buf[i].codr = "";

                    if(!packjson["rxpk"][i]["lsnr"].is_null())
                        packet.rxpk_buf[i].lsnr = packjson["rxpk"][i]["lsnr"];
                    else
                        packet.rxpk_buf[i].lsnr = std::numeric_limits<float>::max();

                    if(!packjson["rxpk"][i]["size"].is_null())
                        packet.rxpk_buf[i].size = packjson["rxpk"][i]["size"];
                    else
                        packet.rxpk_buf[i].size = std::numeric_limits<uint32_t>::max();

                    if(!packjson["rxpk"][i]["data"].is_null()) {
                        std::string data = packjson["rxpk"][i]["data"];
                        packet.rxpk_buf[i].data = packjson["rxpk"][i]["data"];
                    }
                    else
                        packet.rxpk_buf[i].data = "";
                }

            } catch (std::exception const& e) {
                if (debug) std::cerr << "Exception processing rxpk: " << e.what() << std::endl;
                return;
            }
        }
    }

      void LoRaPacketParser::parsePullJson(const char *string, LoRaPacket& packet, bool debug) {
        json packjson;
        try {
            packjson = json::parse(string);
        } catch (const std::exception& e) {
            if (debug) std::cerr << "JSON parse error in pull data: " << e.what() << std::endl;
            return;
        }

        if(!packjson["txpk"].is_null()) {
            if(debug) std::cout << "txpk: " << packjson["txpk"] << std::endl;

            packet.txpk_buf = std::make_unique<txpk>();

            if(packjson["txpk"]["imme"].is_null())
                packet.txpk_buf->imme = false;
            else
                packet.txpk_buf->imme = packjson["txpk"]["imme"];

            if(packjson["txpk"]["tmst"].is_null())
                packet.txpk_buf->tmst = std::numeric_limits<uint64_t>::max();
            else
                packet.txpk_buf->tmst = packjson["txpk"]["tmst"];

            if(packjson["txpk"]["tmms"].is_null())
                packet.txpk_buf->tmms = std::numeric_limits<uint64_t>::max();
            else
                packet.txpk_buf->tmms = packjson["txpk"]["tmms"];

            if(packjson["txpk"]["freq"].is_null())
                packet.txpk_buf->freq = std::numeric_limits<float>::max();
            else
                packet.txpk_buf->freq = packjson["txpk"]["freq"];

            if(packjson["txpk"]["rfch"].is_null())
                packet.txpk_buf->rfch = std::numeric_limits<uint32_t>::max();
            else
                packet.txpk_buf->rfch = packjson["txpk"]["rfch"];

            if(packjson["txpk"]["powe"].is_null())
                packet.txpk_buf->powe = std::numeric_limits<uint32_t>::max();
            else
                packet.txpk_buf->powe = packjson["txpk"]["powe"];

            if(packjson["txpk"]["modu"].is_null())
                packet.txpk_buf->modu = "";
            else
                packet.txpk_buf->modu = packjson["txpk"]["modu"];

            if(packjson["txpk"]["datr"].is_null())
                packet.txpk_buf->datr = "";
            else
                packet.txpk_buf->datr = packjson["txpk"]["datr"];

            if(packjson["txpk"]["codr"].is_null())
                packet.txpk_buf->codr = "";
            else
                packet.txpk_buf->codr = packjson["txpk"]["codr"];
            
            if(packjson["txpk"]["fdev"].is_null())
                packet.txpk_buf->fdev = std::numeric_limits<uint32_t>::max();
            else
                packet.txpk_buf->fdev = packjson["txpk"]["fdev"];

            if(packjson["txpk"]["ipol"].is_null())
                packet.txpk_buf->ipol = false;
            else
                packet.txpk_buf->ipol = packjson["txpk"]["ipol"];

            if(packjson["txpk"]["prea"].is_null())
                packet.txpk_buf->prea = std::numeric_limits<uint32_t>::max();
            else
                packet.txpk_buf->prea = packjson["txpk"]["prea"];

            if(packjson["txpk"]["size"].is_null())
                packet.txpk_buf->size = std::numeric_limits<uint32_t>::max();
            else
                packet.txpk_buf->size = packjson["txpk"]["size"];

            if(packjson["txpk"]["data"].is_null())
                packet.txpk_buf->data = "";
            else
                packet.txpk_buf->data = packjson["txpk"]["data"];

            if(packjson["txpk"]["ncrc"].is_null())
                packet.txpk_buf->ncrc = false;
            else
                packet.txpk_buf->ncrc = packjson["txpk"]["ncrc"];
        }
    }
}