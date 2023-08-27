#include <iostream>
#include <cstring>

#include "database.hpp"

namespace lorareforwarder::database {

    Database::Database(const std::string &host_name, const std::string &user_name, const std::string &password, 
                       const std::string &schema, uint32_t port, bool _debug) {
        try {
            sql::Driver* driver = sql::mariadb::get_driver_instance();

            sql::SQLString url("jdbc:mariadb://" + host_name + ":" + std::to_string(port) + "/" + schema);
            std::cout << "URL: " << url << std::endl;
            sql::Properties properties({{"user", user_name}, {"password", password}, {"autoReconnect", "true"}});
            std::cout << "Dati: " << user_name << " " << password << std::endl;

            conn = Database::generateConnection(driver, url, properties);

            connected = true;
            debug = _debug;
        } catch(sql::SQLException& e){
            std::cerr << "Error when enstablishing connection: " << e.what() << std::endl;
            exit(-1);
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

        int64_t last_id = 0;
        int32_t res;

        if(packet->rxpk_buf.size() > 0) {
            if(debug) std::cout << "Adding a new push packet to the database" << std::endl;
            
            for(int i = 0; i < packet->rxpk_buf.size(); i++) {
                if(debug) std::cout << "Adding a new push packet to the database" << std::endl;

                try {
                    std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement("INSERT INTO upPackets (gwui, time, tmms, tmst, freq, chan, rfch, stat, modu, datr, codr, rssi, lsnr, size, data) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

                    stmnt->setString(1, macStr);
                    stmnt->setString(2, packet->rxpk_buf[i].time.c_str());
                    stmnt->setUInt64(3, packet->rxpk_buf[i].tmms);
                    stmnt->setUInt(4, packet->rxpk_buf[i].tmst);
                    stmnt->setFloat(5, packet->rxpk_buf[i].freq);
                    stmnt->setUInt(6, packet->rxpk_buf[i].chan);
                    stmnt->setUInt(7, packet->rxpk_buf[i].rfch);
                    stmnt->setShort(8, packet->rxpk_buf[i].stat);
                    stmnt->setString(9, packet->rxpk_buf[i].modu.c_str());
                    stmnt->setString(10, packet->rxpk_buf[i].datr.c_str());
                    stmnt->setString(11, packet->rxpk_buf[i].codr.c_str());
                    stmnt->setInt(12, packet->rxpk_buf[i].rssi);
                    stmnt->setFloat(13, packet->rxpk_buf[i].lsnr);
                    stmnt->setUInt(14, packet->rxpk_buf[i].size);
                    stmnt->setString(15, packet->rxpk_buf[i].data.c_str());
                    
                    //stmnt->setNull(16, sql::_NULL);

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
                std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement("INSERT INTO downPackets (imme, tmst, tmms, freq, rfch, powe, modu, datr, codr, fdev, ipol, prea, size, data, ncrc) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

                stmnt->setBoolean(1, packet->txpk_buf->imme);
                stmnt->setUInt64(2, packet->txpk_buf->tmst);
                stmnt->setUInt64(3, packet->txpk_buf->tmms);
                stmnt->setFloat(4, packet->txpk_buf->freq);
                stmnt->setUInt(5, packet->txpk_buf->rfch);
                stmnt->setUInt(6, packet->txpk_buf->powe);
                stmnt->setString(7, packet->txpk_buf->modu.c_str());
                stmnt->setString(8, packet->txpk_buf->datr.c_str());
                stmnt->setString(9, packet->txpk_buf->codr.c_str());
                stmnt->setUInt(10, packet->txpk_buf->fdev);
                stmnt->setBoolean(11, packet->txpk_buf->ipol);
                stmnt->setUInt(12, packet->txpk_buf->prea);
                stmnt->setUInt(13, packet->txpk_buf->size);
                stmnt->setString(14, packet->txpk_buf->data.c_str());
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