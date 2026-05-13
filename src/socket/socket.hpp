#ifndef LORAREFORWARDER_SOCKET_HPP
#define LORAREFORWARDER_SOCKET_HPP

#include "base.hpp"

#include <string>
#include <memory>
#include <thread>

class UDPSocket : public BaseSocket {
    public:
        std::function<void(const uint8_t*, int, std::string, std::uint16_t)> onRawMessageReceived;

        explicit UDPSocket(FDR_ON_ERROR, int socketId = -1): BaseSocket(onError, socketId) {
        }

        void SendTo(const uint8_t *bytes, size_t byteslength, std::string host, uint16_t port) {
            sockaddr_in hostAddr;
            memset(&hostAddr, 0, sizeof(hostAddr));

            struct addrinfo hints, *res, *it;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_DGRAM;

            int status;
            if ((status = getaddrinfo(host.c_str(), NULL, &hints, &res)) != 0) {
                onError(errno, "Invalid address." + std::string(gai_strerror(status)));
                return;
            }

            bool found = false;
            for (it = res; it != NULL; it = it->ai_next) {
                if (it->ai_family == AF_INET) { // IPv4
                    memcpy((void *)(&hostAddr), (void *)it->ai_addr, sizeof(sockaddr_in));
                    found = true;
                    break;
                }
            }

            freeaddrinfo(res);

            if (!found) {
                onError(errno, "No IPv4 address found for host.");
                return;
            }

            hostAddr.sin_port = htons(port);
            hostAddr.sin_family = AF_INET;

            if (sendto(this->sock, bytes, byteslength, 0, (sockaddr *)&hostAddr, sizeof(hostAddr)) < 0) {
                onError(errno, "Cannot send message to the address.");
                return;
            }
        }

        void run() {
            sockaddr_in hostAddr;
            socklen_t hostAddrSize = sizeof(hostAddr);

            uint8_t* tempBuffer = new uint8_t[BUFFER_SIZE];

            while (!getShouldShutdown()) {
                int messageLength;
                messageLength = recvfrom(sock, tempBuffer, BUFFER_SIZE - 1, 0, (sockaddr *)&hostAddr, &hostAddrSize);

                if (messageLength < 0) {
                    perror("recvfrom");
                    delete[] tempBuffer;
                    return;
               } else {
                    int safeLength = messageLength;
                    if (safeLength >= static_cast<int>(BUFFER_SIZE)) {
                        safeLength = BUFFER_SIZE - 1;
                    }
                    tempBuffer[safeLength] = '\0';
                    if (onRawMessageReceived) onRawMessageReceived(tempBuffer, messageLength, ipToString(hostAddr), ntohs(hostAddr.sin_port));
                }
            }

            delete[] tempBuffer;
        }
};

#endif /* LORAREFORWARDER_SOCKET_HPP */
