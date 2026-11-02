#pragma once

#include <ostream>
#include <stdexcept>
#include <string>

// Thrown when the config file is missing, wrong, or has invalid values.
class ConfigError : public std::runtime_error {
public:
    explicit ConfigError(const std::string& message) : std::runtime_error(message) {}

    friend std::ostream& operator<<(std::ostream& os, const ConfigError& error) {
        os << "config error: " << error.what();
        return os;
    }
};
