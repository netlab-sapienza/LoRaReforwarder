#include <iostream>
#include <cstring>
#include <mariadb++/result_set.hpp>
#include <mariadb++/statement.hpp>

#include "database.hpp"

namespace lorareforwarder::database {

    Database::Database(const std::string &host_name, const std::string &user_name, const std::string &password, 
                       const std::string &schema, mariadb::u32 port) {

        acc = mariadb::account::create(host_name, user_name, password, schema, port);
        con = mariadb::connection::create(acc);
    }

    Database::~Database() = default;

    int Database::insertPushDataPacket(packet::LoRaPacket* packet) {

        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%08X%08X", (uint32_t)(packet->gw_mac >> 32), (uint32_t)(packet->gw_mac & 0xFFFFFFFF));
        std::string str(macStr);

        std::cout << "CIao: " << str << std::endl;


        mariadb::s64 statid = -1;
        if(packet->stat_buf) {
            mariadb::statement_ref stmt = con->create_statement("INSERT INTO pushStats (id, time, lati, longi, alti, rxnb, rxok, rxfw, ackr, dwnb, txnb) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        
            stmt->set_string(1, packet->stat_buf->time);
            stmt->set_float(2, packet->stat_buf->lati);
            stmt->set_float(3, packet->stat_buf->longi);
            stmt->set_signed32(4, packet->stat_buf->alti);
            stmt->set_unsigned32(5, packet->stat_buf->rxnb);
            stmt->set_signed32(6, packet->stat_buf->rxok);
            stmt->set_unsigned32(7, packet->stat_buf->rxfw);
            stmt->set_signed32(8, packet->stat_buf->ackr);
            stmt->set_unsigned32(9, packet->stat_buf->dwnb);
            stmt->set_unsigned32(10, packet->stat_buf->txnb);
            statid = stmt->insert();

            std::cout << "Inserito: " << statid << std::endl;
        }

        mariadb::statement_ref stmt = con->create_statement("INSERT INTO pushPackets (id, gwui, time, tmms, tmst, freq, chan, rfch, stat, modu, datrid, datr, codr, rssi, lsnr, size, data, statId) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

        stmt->set_string(1, macStr);
        stmt->set_string(2, packet->rxpk_buf->time);
        stmt->set_unsigned64(3, packet->rxpk_buf->tmms);
        stmt->set_unsigned32(4, packet->rxpk_buf->tmst);
        stmt->set_float(5, packet->rxpk_buf->freq);
        stmt->set_unsigned32(6, packet->rxpk_buf->chan);
        stmt->set_unsigned32(7, packet->rxpk_buf->rfch);
        stmt->set_signed16(8, packet->rxpk_buf->stat); // DA VERIFICARE
        stmt->set_string(9, packet->rxpk_buf->modu);
        stmt->set_string(10, packet->rxpk_buf->datrid);
        stmt->set_unsigned32(11, packet->rxpk_buf->datr);
        stmt->set_string(12, packet->rxpk_buf->codr);
        stmt->set_signed32(13, packet->rxpk_buf->rssi);
        stmt->set_float(14, packet->rxpk_buf->lsnr);
        stmt->set_unsigned32(15, packet->rxpk_buf->size);
        stmt->set_string(16, packet->rxpk_buf->data);
        if(statid > -1) stmt->set_unsigned64(17, statid);
        stmt->insert();

        return 0;
    }
}