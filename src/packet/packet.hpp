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
#include <mariadb++/types.hpp>

namespace lorareforwarder::packet {
    enum Modulation {LORA, FSK};

    typedef struct {
        std::string time;
        mariadb::u64 tmms;
        mariadb::u32 tmst;
        mariadb::f32 freq;
        mariadb::u32 chan;
        mariadb::u32 rfch;
        mariadb::u32 stat;
        std::string modu;
        std::string datrid;
        mariadb::u32 datr;
        std::string codr;
        mariadb::s32 rssi;
        mariadb::f32 lsnr;
        mariadb::u32 size;
        std::string data;
    } rxpk;

    typedef struct {
        std::string time;
        mariadb::f32 lati;
        mariadb::f32 longi;
        mariadb::s32 alti;
        mariadb::u32 rxnb;
        mariadb::s32 rxok;
        mariadb::u32 rxfw;
        mariadb::s32 ackr;
        mariadb::u32 dwnb;
        mariadb::u32 txnb;
    } stat;

    class LoRaPacket {
        public:
            explicit LoRaPacket() = default;
            ~LoRaPacket() = default;

            const uint8_t *buf;
            size_t length;

            uint32_t raw_mac_h; /* Most Significant Nibble, network order */
            uint32_t raw_mac_l; /* Least Significant Nibble, network order */
            uint64_t gw_mac; /* MAC address of the client (gateway) */

            uint8_t ack_command;

            rxpk** rxpk_buf;
            stat* stat_buf;

            bool isValid = true;
    };
}

#endif /* LORAREFORWARDER_PACKET_HPP */