#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <stdexcept>

namespace ebike {

using Tick = std::uint32_t;
using Priority = std::uint8_t;

class ConfigError : public std::runtime_error {
public:
    explicit ConfigError(const std::string& message) : std::runtime_error(message) {}
};

class Format {
public:
    Format() = delete;

    static std::string fixed(double value, int digits) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(digits) << value;
        return out.str();
    }

    static std::string percent(double ratio) { return fixed(ratio * 100.0, 1) + "%"; }
};

} 

#endif
