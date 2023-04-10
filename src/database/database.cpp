#include <iostream>
#include <cstring>

#include "database.hpp"

namespace lorareforwarder::database {

    Database::Database(const std::string &host_name, const std::string &user_name, const std::string &password, 
                       const std::string &schema, uint32_t port) {
        try {
            sql::Driver* driver = sql::mariadb::get_driver_instance();

            sql::SQLString url("jdbc:mariadb://" + host_name + ":" + std::to_string(port) + "/" + schema);
            sql::Properties properties({{"user", user_name}, {"password", password}});

            conn = Database::generateConnection(driver, url, properties);
        } catch(sql::SQLException& e){
            std::cerr << "Error when enstablishing connection: " << e.what() << std::endl;
        }
    }

    std::unique_ptr<sql::Connection> Database::generateConnection(sql::Driver* driver, const sql::SQLString &url, sql::Properties &props) {
        std::unique_ptr<sql::Connection> localconn(driver->connect(url, props));
        return localconn;
    }

    Database::~Database() = default;

    int Database::insertPushDataPacket(packet::LoRaPacket* packet) {
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%08X%08X", (uint32_t)(packet->gw_mac >> 32), (uint32_t)(packet->gw_mac & 0xFFFFFFFF));
        std::string str(macStr);

        sql::ResultSet *res;
        if(packet->stat_buf) {
            std::cout << "Adding a new packet to the database" << std::endl;

            try {
                std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement("INSERT INTO pushStats (id, time, lati, longi, alti, rxnb, rxok, rxfw, ackr, dwnb, txnb) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

                stmnt->setString(1, packet->stat_buf->time);
                stmnt->setFloat(2, packet->stat_buf->lati);
                stmnt->setFloat(3, packet->stat_buf->longi);
                stmnt->setInt(4, packet->stat_buf->alti);
                stmnt->setUInt(5, packet->stat_buf->rxnb);
                stmnt->setInt(6, packet->stat_buf->rxok);
                stmnt->setUInt(7, packet->stat_buf->rxfw);
                stmnt->setInt(8, packet->stat_buf->ackr);
                stmnt->setUInt(9, packet->stat_buf->dwnb);
                stmnt->setUInt(10, packet->stat_buf->txnb);
                res = stmnt->executeQuery();

                std::cout << "Inserito: " << res->getMetaData()->getColumnName(0) << std::endl;
            } catch(sql::SQLException& e){
                std::cerr << "Error inserting new packet: " << e.what() << std::endl;
            }
        }

        /*if(packet->rxpk_buf) {
            mariadb::statement_ref stmt = con->create_statement("INSERT INTO pushPackets (id, gwui, time, tmms, tmst, freq, chan, rfch, stat, modu, datrid, datr, codr, rssi, lsnr, size, data, statId) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

            for(int i = 0; i < sizeof(packet->rxpk_buf)/sizeof(packet->rxpk_buf[0]); i++) {
                stmt->set_string(1, macStr);
                stmt->set_string(2, packet->rxpk_buf[i]->time);
                stmt->set_unsigned64(3, packet->rxpk_buf[i]->tmms);
                stmt->set_unsigned32(4, packet->rxpk_buf[i]->tmst);
                stmt->set_float(5, packet->rxpk_buf[i]->freq);
                stmt->set_unsigned32(6, packet->rxpk_buf[i]->chan);
                stmt->set_unsigned32(7, packet->rxpk_buf[i]->rfch);
                stmt->set_signed16(8, packet->rxpk_buf[i]->stat);
                stmt->set_string(9, packet->rxpk_buf[i]->modu);
                stmt->set_string(10, packet->rxpk_buf[i]->datrid);
                stmt->set_unsigned32(11, packet->rxpk_buf[i]->datr);
                stmt->set_string(12, packet->rxpk_buf[i]->codr);
                stmt->set_signed32(13, packet->rxpk_buf[i]->rssi);
                stmt->set_float(14, packet->rxpk_buf[i]->lsnr);
                stmt->set_unsigned32(15, packet->rxpk_buf[i]->size);
                stmt->set_string(16, packet->rxpk_buf[i]->data);

                if(statid > -1) stmt->set_unsigned64(17, statid);
                else stmt->set_unsigned64(17, mariadb::value::null);
                stmt->insert();

                free(packet->rxpk_buf[0]);
            }

            free(packet->rxpk_buf);
            return 0;
        }*/

        free(packet);
        return -1;
    }

    int Database::insertPullDataPacket(packet::LoRaPacket* packet) {
        /*if(packet->txpk_buf) {
            mariadb::statement_ref stmt = con->create_statement("INSERT INTO pullPackets (id, imme, tmst, tmms, freq, rfch, powe, modu, datrid, datr, codr, fdev, ipol, prea, size, data, ncrc) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

            stmt->set_boolean(1, packet->txpk_buf->imme);
            stmt->set_unsigned64(2, packet->txpk_buf->tmst);
            stmt->set_unsigned64(3, packet->txpk_buf->tmms);
            stmt->set_float(4, packet->txpk_buf->freq);
            stmt->set_unsigned32(5, packet->txpk_buf->rfch);
            stmt->set_unsigned32(6, packet->txpk_buf->powe);
            stmt->set_string(7, packet->txpk_buf->modu);
            stmt->set_string(8, packet->txpk_buf->datrid);
            stmt->set_unsigned32(9, packet->txpk_buf->datr);
            stmt->set_string(10, packet->txpk_buf->codr);
            stmt->set_unsigned32(11, packet->txpk_buf->fdev);
            stmt->set_boolean(12, packet->txpk_buf->ipol);
            stmt->set_unsigned32(13, packet->txpk_buf->prea);
            stmt->set_unsigned32(14, packet->txpk_buf->size);
            stmt->set_string(15, packet->txpk_buf->data);
            stmt->set_boolean(16, packet->txpk_buf->ncrc);
            stmt->insert();

            free(packet->txpk_buf);
            return 0;
        }

        free(packet);*/
        return -1;
    }
}