#include "Config.hpp"

#include <cmath>
#include <limits>
#include <ostream>

#include "ConfigError.hpp"

Config::Config(const std::map<std::string, double>& values, const std::vector<Task>& tasks)
    : values_(values), tasks_(tasks) {}

double Config::number(const std::string& key) const {
    const auto found = values_.find(key);
    if (found == values_.end()) {
        throw ConfigError("missing config key '" + key + "'");
    }
    return found->second;
}

int Config::whole(const std::string& key) const {
    const double value = number(key);
    const double biggest = std::numeric_limits<int>::max();
    if (value < 0 || value > biggest || value != std::floor(value)) {
        throw ConfigError("config key '" + key + "' must be a whole number between 0 and " +
                          std::to_string(std::numeric_limits<int>::max()));
    }
    return static_cast<int>(value);
}

std::ostream& operator<<(std::ostream& os, const Config& config) {
    os << "Config{" << config.values_.size() << " values, " << config.tasks_.size() << " tasks}";
    for (const Task& task : config.tasks_) {
        os << "\n  " << task;
    }
    return os;
}
