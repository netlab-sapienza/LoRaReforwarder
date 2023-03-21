#ifndef LORAREFORWARDER_COMMON_HPP
#define LORAREFORWARDER_COMMON_HPP

#include <sys/socket.h>
#include <arpa/inet.h>

#define handle_error_en(_errno, msg)    do { errno = _errno; perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error(msg)           do { perror(msg); exit(EXIT_FAILURE); } while (0)

namespace lorareforwarder::common {
    typedef struct handler_args_s {
        unsigned int reqno;
        int socket_desc;
        struct sockaddr_in* client_addr;
    } handler_args_t;
}

#endif /* LORAREFORWARDER_COMMON_HPP */