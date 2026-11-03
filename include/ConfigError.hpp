#pragma once

#include <string>

#include "EbikeError.hpp"

// Thrown when the config file is missing, wrong, or has invalid values.
class ConfigError : public EbikeError {
public:
    explicit ConfigError(const std::string& message) : EbikeError(message) {}

    std::string category() const override { return "config"; }
};
