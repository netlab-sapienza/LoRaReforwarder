#include "common.hpp"
#include <iostream>
#include <netdb.h>

namespace lorareforwarder::common {
    void handle_error(std::string errorMsg) {
        std::cout << "Runtime Error: " << errorMsg << std::endl;
        exit(EXIT_FAILURE);
    }

    void handle_error_en(int _errno, std::string errorMsg) {
        std::cout << "Runtime Error, Number: " << _errno << " - " << errorMsg << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string hostToIp(const std::string& host) {
        struct addrinfo hints, *res = nullptr;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        int status = getaddrinfo(host.c_str(), nullptr, &hints, &res);
        if (status != 0 || res == nullptr) {
            return {};
        }

        std::string ip;
        struct sockaddr_in* ipv4 = nullptr;
        for (struct addrinfo* it = res; it != nullptr; it = it->ai_next) {
            if (it->ai_family == AF_INET) {
                ipv4 = reinterpret_cast<struct sockaddr_in*>(it->ai_addr);
                break;
            }
        }

        if (ipv4) {
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &ipv4->sin_addr, ipStr, INET_ADDRSTRLEN);
            ip = std::string(ipStr);
        }

        freeaddrinfo(res);
        return ip;
    }
}