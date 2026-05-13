#ifndef LORAREFORWARDER_SOCKETBASE_HPP
#define LORAREFORWARDER_SOCKETBASE_HPP

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <functional>
#include <cerrno>
#include <string>


class BaseSocket {
    public:
        const uint16_t BUFFER_SIZE = 0xFFFF;
        sockaddr_in address;
        bool isClosed = false;
        #define FDR_UNUSED(expr){ (void)(expr); }
        #define FDR_ON_ERROR std::function<void(int, std::string)> onError = [](int errorCode, std::string errorMessage){FDR_UNUSED(errorCode); FDR_UNUSED(errorMessage)}

    protected:
        int sock = 0;
        bool shouldShutdown = false;

    public:
        void setShouldShutdown(bool val) { shouldShutdown = val; }
        bool getShouldShutdown() const { return shouldShutdown; }

        static std::string ipToString(sockaddr_in addr) {
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);

            return std::string(ip);
        }

        BaseSocket(FDR_ON_ERROR, int socketId = -1) {
            if (socketId < 0) {
                if ((this->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                    onError(errno, "Socket creating error.");
                }
            } else {
                this->sock = socketId;
            }
        }

    public:
        virtual ~BaseSocket() {
            socketClose();
        }

        virtual void socketClose() {
            if(isClosed) return;
            isClosed = true;
            close(this->sock);
        }

        std::string getRemoteAddress() {
            return ipToString(this->address);
        }

        int getRemotePort() {
            return ntohs(this->address.sin_port);
        }

        int getFileDescriptor() const {
            return this->sock;
        }
};

#endif /* LORAREFORWARDER_SOCKETBASE_HPP */
