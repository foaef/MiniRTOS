#ifndef CONFIG_H
#define CONFIG_H

#include "Common.h"
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <istream>
#include <optional>

namespace ebike {

class TaskSpec {
    std::string name_;
    std::string type_;
    Priority priority_;
    Tick period_;
    Tick wcet_;
    Tick deadline_;
    bool usesBus_;

public:
    TaskSpec(std::string name, std::string type, Priority priority, Tick period, Tick wcet, Tick deadline,
             bool usesBus);
    
    const std::string& name() const { return name_; }
    const std::string& type() const { return type_; }
    Priority priority() const { return priority_; }
    Tick period() const { return period_; }
    Tick wcet() const { return wcet_; }
    Tick deadline() const { return deadline_; }
    bool usesBus() const { return usesBus_; }
    double utilization() const { return static_cast<double>(wcet_) / static_cast<double>(period_); }

    friend std::ostream& operator<<(std::ostream& os, const TaskSpec& spec);
};

class TimerSpec {
    std::string name_;
    Tick period_;
    std::string action_;

public:
    TimerSpec(std::string name, Tick period, std::string action);
    
    const std::string& name() const { return name_; }
    Tick period() const { return period_; }
    const std::string& action() const { return action_; }

    friend std::ostream& operator<<(std::ostream& os, const TimerSpec& spec);
};

class BikeConfig {
    std::map<std::string, double> values_;
    std::vector<TaskSpec> tasks_;
    std::vector<TimerSpec> timers_;

public:
    void setValue(const std::string& key, double value) { values_[key] = value; }
    void addTask(TaskSpec spec) { tasks_.push_back(std::move(spec)); }
    void addTimer(TimerSpec spec) { timers_.push_back(std::move(spec)); }

    double number(const std::string& key) const;
    Tick ticks(const std::string& key) const;

    const std::vector<TaskSpec>& tasks() const { return tasks_; }
    const std::vector<TimerSpec>& timers() const { return timers_; }

    double totalUtilization() const;

    friend std::ostream& operator<<(std::ostream& os, const BikeConfig& config);
};

class ConfigLoader {
public:
    static BikeConfig fromFile(const std::filesystem::path& path);
    static BikeConfig fromStream(std::istream& in, const std::string& origin);

private:
    static std::string trim(const std::string& text);
    static void parseLine(const std::string& line, BikeConfig& config);
    static void parseAssignment(const std::string& line, BikeConfig& config);
    static TaskSpec parseTask(std::istringstream& words);
    static TimerSpec parseTimer(std::istringstream& words);
    static double parseNumber(const std::string& text);
    static std::uint32_t parseWhole(const std::string& text, std::uint32_t maximum);
};

class ConfigLocator {
public:
    static std::optional<std::filesystem::path> locate(const std::vector<std::string>& args);
};

}

#endif
