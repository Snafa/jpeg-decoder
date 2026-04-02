#pragma once

#include <stdexcept>
#include <string>

[[noreturn]] inline void Error(const std::string& message) {
    throw std::runtime_error(message);
}

inline void Require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::invalid_argument(message);
    }
}