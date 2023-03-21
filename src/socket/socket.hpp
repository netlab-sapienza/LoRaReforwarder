#pragma once

#include "base.hpp"
#include <string.h>
#include <thread>
#include <future>

class UDPSocket : public BaseSocket {
    public:
        std::function<void(const uint8_t*, int, std::string, std::uint16_t)> onRawMessageReceived;

        explicit UDPSocket(bool isServer = true, FDR_ON_ERROR, int socketId = -1): BaseSocket(onError, socketId) {
            this->isServer = isServer;

            std::thread t(ReceiveFrom, this);
            t.detach();
        }
        
        void SendTo(const uint8_t *bytes, size_t byteslength, std::string host, uint16_t port, FDR_ON_ERROR) {
            sockaddr_in hostAddr;

            struct addrinfo hints, *res, *it;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_DGRAM;

            int status;
            if ((status = getaddrinfo(host.c_str(), NULL, &hints, &res)) != 0) {
                onError(errno, "Invalid address." + std::string(gai_strerror(status)));
                return;
            }

            for (it = res; it != NULL; it = it->ai_next) {
                if (it->ai_family == AF_INET) { // IPv4
                    memcpy((void *)(&hostAddr), (void *)it->ai_addr, sizeof(sockaddr_in));
                    break; // for now, just get first ip (ipv4).
                }
            }

            freeaddrinfo(res);

            hostAddr.sin_port = htons(port);
            hostAddr.sin_family = AF_INET;

            if (sendto(this->sock, bytes, byteslength, 0, (sockaddr *)&hostAddr, sizeof(hostAddr)) < 0) {
                onError(errno, "Cannot send message to the address.");
                return;
            }
        }

    private:
        bool isServer = true;
        static void ReceiveFrom(UDPSocket *udpSocket) {
            sockaddr_in hostAddr;
            socklen_t hostAddrSize = sizeof(hostAddr);

            uint8_t tempBuffer[udpSocket->BUFFER_SIZE];

            while (true) {
                int messageLength;
                messageLength = recvfrom(udpSocket->sock, tempBuffer, udpSocket->BUFFER_SIZE, 0, (sockaddr *)&hostAddr, &hostAddrSize);

                if (messageLength < 0) {
                    perror("recvfrom");
                    return;
                }
                else {
                    tempBuffer[messageLength] = '\0';

                    if (udpSocket->onRawMessageReceived) {
                        if(udpSocket->isServer) {
                            std::thread t1([&] {
                                udpSocket->onRawMessageReceived(tempBuffer, messageLength, ipToString(hostAddr), ntohs(hostAddr.sin_port));
                            });
                            t1.detach();
                            //udpSocket->onRawMessageReceived(tempBuffer, messageLength, ipToString(hostAddr), ntohs(hostAddr.sin_port));
                        } else {
                            udpSocket->onRawMessageReceived(tempBuffer, messageLength, ipToString(hostAddr), ntohs(hostAddr.sin_port));
                        }
                    }
                }
            }
        }
};