#pragma once

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "Task.hpp"

class Config {
    std::map<std::string, double> values_;
    std::vector<Task> tasks_;

public:
    Config(const std::map<std::string, double>& values, const std::vector<Task>& tasks);

    double number(const std::string& key) const;

    int whole(const std::string& key) const;

    const std::vector<Task>& tasks() const { return tasks_; }

    friend std::ostream& operator<<(std::ostream& os, const Config& config);
};
