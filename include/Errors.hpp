#pragma once

#include <stdexcept>
#include <string>

// Base of all errors of this project. main catches RtosError to report any of them.
class RtosError : public std::runtime_error {
public:
    explicit RtosError(const std::string& message) : std::runtime_error(message) {}
};

// A data file (config or ride) is missing, malformed or has impossible values.
class ConfigError : public RtosError {
public:
    explicit ConfigError(const std::string& message) : RtosError(message) {}
};

// The task set was described in a way the scheduler cannot work with.
class SchedulingError : public RtosError {
public:
    explicit SchedulingError(const std::string& message) : RtosError(message) {}
};

// The firmware asked the hardware for something it cannot do (a firmware bug).
class HardwareError : public RtosError {
public:
    explicit HardwareError(const std::string& message) : RtosError(message) {}
};
