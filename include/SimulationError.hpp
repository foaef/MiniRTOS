#pragma once

#include <string>

#include "EbikeError.hpp"

// Thrown when the simulation is used in a wrong way (for example, no data to
// report yet, or a result file that cannot be written).
class SimulationError : public EbikeError {
public:
    explicit SimulationError(const std::string& message) : EbikeError(message) {}

    std::string category() const override { return "simulation"; }
};
