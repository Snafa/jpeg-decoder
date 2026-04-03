#pragma once

#include <stdexcept>
#include <string>

namespace jpeg_decoder {
    [[noreturn]] inline void Error(const std::string &message) {
        throw std::runtime_error(message);
    }

    inline void Require(const bool condition, const std::string &message) {
        if (!condition) {
            throw std::invalid_argument(message);
        }
    }
}
