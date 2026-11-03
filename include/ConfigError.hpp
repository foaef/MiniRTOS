#pragma once

#include <string>

#include "EbikeError.hpp"

class ConfigError : public EbikeError {
public:
    explicit ConfigError(const std::string& message) : EbikeError(message) {}

    std::string category() const override { return "config"; }
};
