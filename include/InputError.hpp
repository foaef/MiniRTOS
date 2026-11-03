#pragma once

#include <string>

#include "EbikeError.hpp"

class InputError : public EbikeError {
public:
    explicit InputError(const std::string& message) : EbikeError(message) {}

    std::string category() const override { return "input"; }
};
