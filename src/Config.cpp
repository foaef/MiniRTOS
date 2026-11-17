#include "Config.hpp"

#include <cmath>
#include <fstream>
#include <ostream>
#include <sstream>
#include <utility>

#include "Errors.hpp"

Config::Config(std::string path) : path_(std::move(path)) {
    forEachLine(path_, [this](const std::string& line) { readLine(line); });
    if (tasks_.empty()) {
        throw ConfigError(path_ + " defines no tasks");
    }
}

void Config::forEachLine(const std::string& path, const std::function<void(const std::string&)>& handleLine) {
    std::ifstream file(path);
    if (!file) {
        throw ConfigError("cannot open '" + path + "'");
    }
    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        lineNumber++;
        const std::size_t comment = line.find('#');
        if (comment != std::string::npos) {
            line.resize(comment);
        }
        if (line.find_first_not_of(" \t\r") == std::string::npos) {
            continue;
        }
        try {
            handleLine(line);
        } catch (const ConfigError& error) {
            throw ConfigError(path + ":" + std::to_string(lineNumber) + ": " + error.what());
        }
    }
}

void Config::readLine(const std::string& line) {
    std::istringstream words(line);
    std::string first;
    words >> first;
    if (first == "task") {
        std::string name;
        std::string type;
        int priority = 0;
        int period = 0;
        int wcet = 0;
        int deadline = 0;
        if (!(words >> name >> type >> priority >> period >> wcet >> deadline)) {
            throw ConfigError("task needs: name type priority period wcet deadline");
        }
        tasks_.emplace_back(name, type, priority, period, wcet, deadline);
        return;
    }

    const std::size_t equals = line.find('=');
    std::string key;
    std::string value;
    std::istringstream keyPart(line.substr(0, equals));
    std::istringstream valuePart(equals == std::string::npos ? "" : line.substr(equals + 1));
    if (equals == std::string::npos || !(keyPart >> key) || !(valuePart >> value)) {
        throw ConfigError("expected 'key = value' or 'task ...'");
    }
    if (!values_.emplace(key, value).second) {
        throw ConfigError("key '" + key + "' is defined twice");
    }
}

const std::string& Config::text(const std::string& key) const {
    const auto found = values_.find(key);
    if (found == values_.end()) {
        throw ConfigError(path_ + ": missing key '" + key + "'");
    }
    return found->second;
}

double Config::number(const std::string& key) const {
    const std::string& raw = text(key);
    std::size_t used = 0;
    double value = 0;
    try {
        value = std::stod(raw, &used);
    } catch (const std::exception&) {
        used = 0;
    }
    if (used != raw.size()) {
        throw ConfigError(path_ + ": key '" + key + "' must be a number, got '" + raw + "'");
    }
    return value;
}

int Config::whole(const std::string& key) const {
    const double value = number(key);
    if (value < 0 || value > 1e9 || value != std::floor(value)) {
        throw ConfigError(path_ + ": key '" + key + "' must be a whole number >= 0");
    }
    return static_cast<int>(value);
}

std::ostream& operator<<(std::ostream& os, const Config& config) {
    os << "Config{" << config.path_ << ", " << config.values_.size() << " values, " << config.tasks_.size()
       << " tasks}";
    for (const TaskSpec& spec : config.tasks_) {
        os << "\n  " << spec;
    }
    return os;
}
