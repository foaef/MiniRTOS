#pragma once

#include <string>

#include "EbikeError.hpp"

// Thrown when a keyboard command is unknown or has missing arguments.
class InputError : public EbikeError {
public:
    explicit InputError(const std::string& message) : EbikeError(message) {}

    std::string category() const override { return "input"; }
};
