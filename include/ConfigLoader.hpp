#pragma once

#include <iosfwd>
#include <map>
#include <sstream>
#include <string>

#include "Config.hpp"
#include "Task.hpp"

// Reads the config file. One statement per line, '#' starts a comment:
//   key = number
//   task <n> <type> <priority> <period> <wcet> <deadline>
class ConfigLoader {
    std::string path_;

    // Handles a line like "motor.max_power_w = 250".
    static void readValue(const std::string& line, std::map<std::string, double>& values);

    // Handles a line like "task brake_monitor check_brake 4 5 1 5".
    // The word "task" was already read, so `words` continues after it.
    static Task readTask(std::istringstream& words);

public:
    explicit ConfigLoader(const std::string& path);

    Config load() const;

    friend std::ostream& operator<<(std::ostream& os, const ConfigLoader& loader);
};
