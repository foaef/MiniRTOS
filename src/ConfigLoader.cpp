#include "ConfigLoader.hpp"

#include <fstream>
#include <map>
#include <ostream>
#include <sstream>
#include <vector>

#include "ConfigError.hpp"

namespace {

void readValue(const std::string& line, std::map<std::string, double>& values) {
    const std::size_t equals = line.find('=');
    if (equals == std::string::npos) {
        throw ConfigError("expected 'key = value', got '" + line + "'");
    }
    std::istringstream keyPart(line.substr(0, equals));
    std::istringstream valuePart(line.substr(equals + 1));
    std::string key;
    double value = 0;
    if (!(keyPart >> key) || !(valuePart >> value)) {
        throw ConfigError("bad line: '" + line + "'");
    }
    values[key] = value;
}

Task readTask(std::istringstream& words) {
    std::string name;
    std::string type;
    int priority = 0;
    int period = 0;
    int wcet = 0;
    int deadline = 0;
    if (!(words >> name >> type >> priority >> period >> wcet >> deadline)) {
        throw ConfigError("task needs: name type priority period wcet deadline");
    }
    return Task(name, type, priority, period, wcet, deadline);
}

}  // namespace

ConfigLoader::ConfigLoader(const std::string& path) : path_(path) {
    if (path_.empty()) {
        throw ConfigError("config file path is empty");
    }
}

Config ConfigLoader::load() const {
    std::ifstream file(path_);
    if (!file) {
        throw ConfigError("cannot open config file '" + path_ + "'");
    }
    std::map<std::string, double> values;
    std::vector<Task> tasks;
    std::string line;
    while (std::getline(file, line)) {
        const std::size_t comment = line.find('#');
        if (comment != std::string::npos) {
            line.resize(comment);
        }
        std::istringstream words(line);
        std::string firstWord;
        if (!(words >> firstWord)) {
            continue;
        }
        if (firstWord == "task") {
            tasks.push_back(readTask(words));
        } else {
            readValue(line, values);
        }
    }
    return Config(values, tasks);
}

std::ostream& operator<<(std::ostream& os, const ConfigLoader& loader) {
    os << "ConfigLoader{" << loader.path_ << "}";
    return os;
}
