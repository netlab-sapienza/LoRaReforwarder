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

    void Database::insertPushDataPacket(packet::LoRaPacket* packet) {
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%08X%08X", (uint32_t)(packet->gw_mac >> 32), (uint32_t)(packet->gw_mac & 0xFFFFFFFF));
        std::string str(macStr);

        int last_id = -1;
        int32_t res;
        if(packet->stat_buf) {
            std::cout << "Adding a new stat packet to the database" << std::endl;

            try {
                std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement("INSERT INTO pushStats (time, lati, longi, alti, rxnb, rxok, rxfw, ackr, dwnb, txnb) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

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
            
                res = stmnt->executeUpdate();

                std::unique_ptr<sql::Statement> stmt(conn->createStatement());
                std::unique_ptr<sql::ResultSet> resid(stmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
                if(resid->next())
                    last_id = resid->getInt64("id");

            } catch(sql::SQLException& e){
                std::cerr << "Error inserting new stat packet: " << e.what() << std::endl;
            }
        }

        if(packet->rxpk_buf.size() > 0) {
            std::cout << "Adding a new push packet to the database" << std::endl;
            
            for(int i = 0; i < packet->rxpk_buf.size(); i++) {
                std::cout << "Adding a new push packet to the database" << std::endl;

                try {
                    std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement("INSERT INTO pushPackets (gwui, time, tmms, tmst, freq, chan, rfch, stat, modu, datr, codr, rssi, lsnr, size, data, statId) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

                    stmnt->setString(1, macStr);
                    stmnt->setString(2, packet->rxpk_buf[i].time);
                    stmnt->setUInt64(3, packet->rxpk_buf[i].tmms);
                    stmnt->setUInt(4, packet->rxpk_buf[i].tmst);
                    stmnt->setFloat(5, packet->rxpk_buf[i].freq);
                    stmnt->setUInt(6, packet->rxpk_buf[i].chan);
                    stmnt->setUInt(7, packet->rxpk_buf[i].rfch);
                    stmnt->setShort(8, packet->rxpk_buf[i].stat);
                    stmnt->setString(9, packet->rxpk_buf[i].modu);
                    stmnt->setString(10, packet->rxpk_buf[i].datr);
                    stmnt->setString(11, packet->rxpk_buf[i].codr);
                    stmnt->setInt(12, packet->rxpk_buf[i].rssi);
                    stmnt->setFloat(13, packet->rxpk_buf[i].lsnr);
                    stmnt->setUInt(14, packet->rxpk_buf[i].size);
                    stmnt->setString(15, packet->rxpk_buf[i].data);

                    if(last_id > -1) stmnt->setUInt64(16, last_id);
                    else stmnt->setUInt64(16, sql::_NULL);

                    res = stmnt->executeUpdate();
                } catch(sql::SQLException& e){
                    std::cerr << "Error inserting new push packet: " << e.what() << std::endl;
                }
            }

            delete packet;
            return;
        }

        delete packet;
        return;
    }

    void Database::insertPullDataPacket(packet::LoRaPacket* packet) {
        if(packet->txpk_buf) {
            std::cout << "Adding a new pull packet to the database" << std::endl;

            try {
                std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement("INSERT INTO pullPackets (imme, tmst, tmms, freq, rfch, powe, modu, datr, codr, fdev, ipol, prea, size, data, ncrc) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

                stmnt->setBoolean(1, packet->txpk_buf->imme);
                stmnt->setUInt64(2, packet->txpk_buf->tmst);
                stmnt->setUInt64(3, packet->txpk_buf->tmms);
                stmnt->setFloat(4, packet->txpk_buf->freq);
                stmnt->setUInt(5, packet->txpk_buf->rfch);
                stmnt->setUInt(6, packet->txpk_buf->powe);
                stmnt->setString(7, packet->txpk_buf->modu);
                stmnt->setString(8, packet->txpk_buf->datr);
                stmnt->setString(9, packet->txpk_buf->codr);
                stmnt->setUInt(10, packet->txpk_buf->fdev);
                stmnt->setBoolean(11, packet->txpk_buf->ipol);
                stmnt->setUInt(12, packet->txpk_buf->prea);
                stmnt->setUInt(13, packet->txpk_buf->size);
                stmnt->setString(14, packet->txpk_buf->data);
                stmnt->setBoolean(15, packet->txpk_buf->ncrc);
                
                stmnt->executeUpdate();
            } catch(sql::SQLException& e){
                std::cerr << "Error inserting new pull packet: " << e.what() << std::endl;
            }

            delete packet;
            return;
        }
    }
}