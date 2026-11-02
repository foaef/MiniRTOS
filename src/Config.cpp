#include "Config.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <limits>

namespace ebike {

TaskSpec::TaskSpec(std::string name, std::string type, Priority priority, Tick period, Tick wcet, Tick deadline,
                   bool usesBus)
    : name_(std::move(name)),
      type_(std::move(type)),
      priority_(priority),
      period_(period),
      wcet_(wcet),
      deadline_(deadline),
      usesBus_(usesBus) {
    if (name_.empty()) {
        throw ConfigError("task without a name");
    }
    if (period_ == 0 || wcet_ == 0) {
        throw ConfigError("task '" + name_ + "': period and wcet must be positive");
    }
    if (wcet_ > deadline_ || deadline_ > period_) {
        throw ConfigError("task '" + name_ + "': must satisfy wcet <= deadline <= period");
    }
}

std::ostream& operator<<(std::ostream& os, const TaskSpec& spec) {
    return os << "TaskSpec{" << spec.name_ << ", type=" << spec.type_ << ", prio=" << static_cast<int>(spec.priority_)
              << ", T=" << spec.period_ << ", C=" << spec.wcet_ << ", D=" << spec.deadline_
              << ", bus=" << (spec.usesBus_ ? "yes" : "no") << "}";
}

TimerSpec::TimerSpec(std::string name, Tick period, std::string action)
    : name_(std::move(name)), period_(period), action_(std::move(action)) {
    if (period_ == 0) {
        throw ConfigError("timer '" + name_ + "': period must be positive");
    }
}

std::ostream& operator<<(std::ostream& os, const TimerSpec& spec) {
    return os << "TimerSpec{" << spec.name_ << ", every=" << spec.period_ << ", action=" << spec.action_ << "}";
}

double BikeConfig::number(const std::string& key) const {
    const auto found = values_.find(key);
    if (found == values_.end()) {
        throw ConfigError("missing config key '" + key + "'");
    }
    return found->second;
}

Tick BikeConfig::ticks(const std::string& key) const {
    const double value = number(key);
    if (value < 0.0 || value != std::floor(value)) {
        throw ConfigError("config key '" + key + "' must be a non-negative integer");
    }
    return static_cast<Tick>(value);
}

double BikeConfig::totalUtilization() const {
    double total = 0.0;
    for (const TaskSpec& spec : tasks_) {
        total += spec.utilization();
    }
    return total;
}

std::ostream& operator<<(std::ostream& os, const BikeConfig& config) {
    os << "BikeConfig{" << config.values_.size() << " values, " << config.tasks_.size() << " tasks, "
       << config.timers_.size() << " timers, U=" << Format::percent(config.totalUtilization()) << "}";
    return os;
}

BikeConfig ConfigLoader::fromFile(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) {
        throw ConfigError("cannot open config file '" + path.string() + "'");
    }
    return fromStream(in, path.string());
}

BikeConfig ConfigLoader::fromStream(std::istream& in, const std::string& origin) {
    BikeConfig config;
    std::string raw;
    std::size_t lineNumber = 0;
    while (std::getline(in, raw)) {
        ++lineNumber;
        const std::string line = trim(raw.substr(0, raw.find('#')));
        if (line.empty()) {
            continue;
        }
        try {
            parseLine(line, config);
        } catch (const ConfigError& error) {
            throw ConfigError(origin + ":" + std::to_string(lineNumber) + ": " + error.what());
        }
    }
    return config;
}

std::string ConfigLoader::trim(const std::string& text) {
    const char* const blanks = " \t\r\n";
    const std::size_t first = text.find_first_not_of(blanks);
    if (first == std::string::npos) {
        return {};
    }
    const std::size_t last = text.find_last_not_of(blanks);
    return text.substr(first, last - first + 1);
}

void ConfigLoader::parseLine(const std::string& line, BikeConfig& config) {
    std::istringstream words(line);
    std::string head;
    words >> head;
    if (head == "task") {
        config.addTask(parseTask(words));
    } else if (head == "timer") {
        config.addTimer(parseTimer(words));
    } else {
        parseAssignment(line, config);
    }
}

void ConfigLoader::parseAssignment(const std::string& line, BikeConfig& config) {
    const std::size_t equals = line.find('=');
    if (equals == std::string::npos) {
        throw ConfigError("expected 'key = value', got '" + line + "'");
    }
    const std::string key = trim(line.substr(0, equals));
    if (key.empty()) {
        throw ConfigError("empty key");
    }
    config.setValue(key, parseNumber(trim(line.substr(equals + 1))));
}

TaskSpec ConfigLoader::parseTask(std::istringstream& words) {
    std::string name;
    std::string type;
    std::string priority;
    std::string period;
    std::string wcet;
    std::string deadline;
    std::string bus;
    if (!(words >> name >> type >> priority >> period >> wcet >> deadline >> bus)) {
        throw ConfigError("task needs: name type priority period wcet deadline uses_bus");
    }
    return TaskSpec(name, type, static_cast<Priority>(parseWhole(priority, 255)), parseWhole(period, UINT32_MAX),
                    parseWhole(wcet, UINT32_MAX), parseWhole(deadline, UINT32_MAX), parseWhole(bus, 1) == 1);
}

TimerSpec ConfigLoader::parseTimer(std::istringstream& words) {
    std::string name;
    std::string period;
    std::string action;
    if (!(words >> name >> period >> action)) {
        throw ConfigError("timer needs: name period action");
    }
    return TimerSpec(name, parseWhole(period, UINT32_MAX), action);
}

double ConfigLoader::parseNumber(const std::string& text) {
    try {
        std::size_t used = 0;
        const double value = std::stod(text, &used);
        if (used != text.size()) {
            throw ConfigError("trailing characters in number '" + text + "'");
        }
        return value;
    } catch (const std::logic_error&) {
        throw ConfigError("not a number: '" + text + "'");
    }
}

std::uint32_t ConfigLoader::parseWhole(const std::string& text, std::uint32_t maximum) {
    const double value = parseNumber(text);
    if (value < 0.0 || value != std::floor(value) || value > static_cast<double>(maximum)) {
        throw ConfigError("expected a whole number in [0, " + std::to_string(maximum) + "], got '" + text + "'");
    }
    return static_cast<std::uint32_t>(value);
}

std::optional<std::filesystem::path> ConfigLocator::locate(const std::vector<std::string>& args) {
    std::vector<std::filesystem::path> candidates;
    if (!args.empty()) {
        candidates.emplace_back(args.front());
    }
    candidates.emplace_back("assets/ebike_config.txt");
    candidates.emplace_back("../assets/ebike_config.txt");
    candidates.emplace_back("../../assets/ebike_config.txt");
    for (const auto& candidate : candidates) {
        if (std::filesystem::is_regular_file(candidate)) {
            return candidate;
        }
    }
    return std::nullopt;
}

}
