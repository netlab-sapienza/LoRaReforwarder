#include <iostream>
#include <cstring>
#include <limits>

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
                    std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement("INSERT INTO upPackets (gwui, time, tmms, tmst, freq, chan, rfch, stat, modu, datr, codr, rssi, lsnr, size, data) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

                    if(macStr == "")
                        stmnt->setNull(1, sql::_NULL);
                    else
                        stmnt->setString(1, macStr);

                    if(packet->rxpk_buf[i].time == "")
                        stmnt->setNull(2, sql::_NULL);
                    else                    
                        stmnt->setString(2, packet->rxpk_buf[i].time.c_str());

                    if(packet->rxpk_buf[i].tmms == std::numeric_limits<uint64_t>::max())
                        stmnt->setNull(3, sql::_NULL);
                    else                    
                        stmnt->setUInt64(3, packet->rxpk_buf[i].tmms);

                    if(packet->rxpk_buf[i].tmst == std::numeric_limits<uint32_t>::max())
                        stmnt->setNull(4, sql::_NULL);
                    else                    
                        stmnt->setUInt(4, packet->rxpk_buf[i].tmst);

                    if(packet->rxpk_buf[i].freq == std::numeric_limits<float>::max())
                        stmnt->setNull(5, sql::_NULL);
                    else                    
                        stmnt->setFloat(5, packet->rxpk_buf[i].freq);

                    if(packet->rxpk_buf[i].chan == std::numeric_limits<uint32_t>::max())
                        stmnt->setNull(6, sql::_NULL);
                    else                    
                        stmnt->setUInt(6, packet->rxpk_buf[i].chan);
                    
                    if(packet->rxpk_buf[i].rfch == std::numeric_limits<uint32_t>::max())
                        stmnt->setNull(7, sql::_NULL);
                    else                     
                        stmnt->setUInt(7, packet->rxpk_buf[i].rfch);

                    if(packet->rxpk_buf[i].stat == std::numeric_limits<uint32_t>::max())
                        stmnt->setNull(8, sql::_NULL);
                    else
                        stmnt->setShort(8, packet->rxpk_buf[i].stat);
                    
                    if(packet->rxpk_buf[i].modu == "")
                        stmnt->setNull(9, sql::_NULL);
                    else
                        stmnt->setString(9, packet->rxpk_buf[i].modu.c_str());

                    if(packet->rxpk_buf[i].datr == "")
                        stmnt->setNull(10, sql::_NULL);
                    else                    
                        stmnt->setString(10, packet->rxpk_buf[i].datr.c_str());
                
                    if(packet->rxpk_buf[i].codr == "")
                        stmnt->setNull(11, sql::_NULL);
                    else    
                        stmnt->setString(11, packet->rxpk_buf[i].codr.c_str());

                    if(packet->rxpk_buf[i].rssi == std::numeric_limits<int32_t>::max())
                        stmnt->setNull(12, sql::_NULL);
                    else                    
                        stmnt->setInt(12, packet->rxpk_buf[i].rssi);

                    if(packet->rxpk_buf[i].lsnr == std::numeric_limits<float>::max())
                        stmnt->setNull(13, sql::_NULL);
                    else                    
                        stmnt->setFloat(13, packet->rxpk_buf[i].lsnr);

                    if(packet->rxpk_buf[i].size == std::numeric_limits<uint32_t>::max())
                        stmnt->setNull(14, sql::_NULL);
                    else                    
                        stmnt->setUInt(14, packet->rxpk_buf[i].size);

                    if(packet->rxpk_buf[i].data == "")
                        stmnt->setNull(15, sql::_NULL);
                    else                    
                        stmnt->setString(15, packet->rxpk_buf[i].data.c_str());
                    
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
                if(packet->txpk_buf->tmst == std::numeric_limits<uint64_t>::max())
                    stmnt->setNull(2, sql::_NULL);
                else
                    stmnt->setUInt64(2, packet->txpk_buf->tmst);

                if(packet->txpk_buf->tmms == std::numeric_limits<uint64_t>::max())
                    stmnt->setNull(3, sql::_NULL);
                else                
                    stmnt->setUInt64(3, packet->txpk_buf->tmms);

                if(packet->txpk_buf->freq == std::numeric_limits<float>::max())
                    stmnt->setNull(4, sql::_NULL);
                else                
                    stmnt->setFloat(4, packet->txpk_buf->freq);

                if(packet->txpk_buf->rfch == std::numeric_limits<uint32_t>::max())
                    stmnt->setNull(5, sql::_NULL);
                else
                    stmnt->setUInt(5, packet->txpk_buf->rfch);

                if(packet->txpk_buf->powe == std::numeric_limits<uint32_t>::max())
                    stmnt->setNull(6, sql::_NULL);
                else                
                    stmnt->setUInt(6, packet->txpk_buf->powe);

                if(packet->txpk_buf->modu == "")
                    stmnt->setNull(7, sql::_NULL);
                else
                    stmnt->setString(7, packet->txpk_buf->modu.c_str());

                if(packet->txpk_buf->datr == "")
                    stmnt->setNull(8, sql::_NULL);
                else
                    stmnt->setString(8, packet->txpk_buf->datr.c_str());
                
                if(packet->txpk_buf->codr == "")
                    stmnt->setNull(9, sql::_NULL);
                else
                    stmnt->setString(9, packet->txpk_buf->codr.c_str());

                if(packet->txpk_buf->fdev == std::numeric_limits<uint32_t>::max())
                    stmnt->setNull(10, sql::_NULL);
                else 
                    stmnt->setUInt(10, packet->txpk_buf->fdev);

                stmnt->setBoolean(11, packet->txpk_buf->ipol);
                
                if(packet->txpk_buf->prea == std::numeric_limits<uint32_t>::max())
                    stmnt->setNull(12, sql::_NULL);
                else
                    stmnt->setUInt(12, packet->txpk_buf->prea);

                if(packet->txpk_buf->size == std::numeric_limits<uint32_t>::max())
                    stmnt->setNull(13, sql::_NULL);
                else               
                    stmnt->setUInt(13, packet->txpk_buf->size);

                if(packet->txpk_buf->data == "")
                    stmnt->setNull(14, sql::_NULL);
                else                
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