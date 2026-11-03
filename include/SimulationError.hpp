#pragma once

#include <string>

#include "EbikeError.hpp"

class SimulationError : public EbikeError {
public:
    explicit SimulationError(const std::string& message) : EbikeError(message) {}

    std::string category() const override { return "simulation"; }
};
