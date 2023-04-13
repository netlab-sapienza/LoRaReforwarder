#ifndef LORAREFORWARDER_COMMON_HPP
#define LORAREFORWARDER_COMMON_HPP

#include <iostream>
#include <cstring>

namespace lorareforwarder::common {
    void handle_error(std::string errorMsg);
    void handle_error_en(int _errno, std::string errorMsg);
}

#endif /* LORAREFORWARDER_COMMON_HPP */