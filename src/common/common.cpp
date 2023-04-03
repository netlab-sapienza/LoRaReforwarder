#include "common.hpp"

namespace lorareforwarder::common {
    void handle_error(std::string errorMsg) {
        std::cout << "Runtime Error: " << errorMsg << std::endl;
        exit(EXIT_FAILURE);
    }

    void handle_error_en(int _errno, std::string errorMsg) {
        std::cout << "Runtime Error, Number: " << _errno << " - " << errorMsg << std::endl;
        exit(EXIT_FAILURE);
    }
}