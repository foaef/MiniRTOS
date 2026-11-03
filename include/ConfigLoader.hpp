#pragma once

#include <iosfwd>
#include <string>

#include "Config.hpp"

// Reads the config file. One statement per line, '#' starts a comment:
//   key = number
//   task <n> <type> <priority> <period> <wcet> <deadline>
class ConfigLoader {
    std::string path_;

public:
    explicit ConfigLoader(const std::string& path);

    Config load() const;

    friend std::ostream& operator<<(std::ostream& os, const ConfigLoader& loader);
};
