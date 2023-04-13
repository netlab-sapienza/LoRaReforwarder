#ifndef LORAREFORWARDER_PACKET_HPP
#define LORAREFORWARDER_PACKET_HPP

#define PKT_PROTOCOL_VERSION 2

#define PKT_PUSH_DATA    0
#define PKT_PUSH_ACK     1
#define PKT_PULL_DATA    2
#define PKT_PULL_RESP    3
#define PKT_PULL_ACK     4
#define PKT_TX_ACK       5

#include <stdlib.h>
#include <mariadb/conncpp/Types.hpp>
namespace lorareforwarder::packet {
    typedef struct {
        bool imme;
        uint64_t tmst;
        uint64_t tmms;
        float freq;
        uint32_t rfch;
        uint32_t powe;
        std::string modu;
        std::string datrid;
        uint32_t datr;
        std::string codr;
        uint32_t fdev;
        bool ipol;
        uint32_t prea;
        uint32_t size;
        std::string data;
        bool ncrc;
    } txpk;

    typedef struct {
        std::string time;
        uint64_t tmms;
        uint32_t tmst;
        float freq;
        uint32_t chan;
        uint32_t rfch;
        uint32_t stat;
        std::string modu;
        std::string datrid;
        std::string codr;
        int32_t rssi;
        float lsnr;
        uint32_t size;
        std::string data;
    } rxpk;

    typedef struct {
        std::string time;
        float lati;
        float longi;
        int32_t alti;
        uint32_t rxnb;
        int32_t rxok;
        uint32_t rxfw;
        int32_t ackr;
        uint32_t dwnb;
        uint32_t txnb;
    } stat;

    class LoRaPacket {
        public:
            explicit LoRaPacket() = default;
            ~LoRaPacket() = default;

            const uint8_t *buf;
            size_t length;
            uint8_t type;

            uint32_t raw_mac_h; /* Most Significant Nibble, network order */
            uint32_t raw_mac_l; /* Least Significant Nibble, network order */
            uint64_t gw_mac; /* MAC address of the client (gateway) */

            uint8_t ack_command;

            rxpk* rxpk_buf;
            txpk* txpk_buf;
            stat* stat_buf;

            bool isValid = true;
    };
}

#endif /* LORAREFORWARDER_PACKET_HPP */