#pragma once

#include <functional>
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "TaskSpec.hpp"

// Everything read from the config file. One statement per line, '#' starts a comment:
//   key = value          (a number, or a text such as a file name)
//   task <name> <type> <priority> <period> <wcet> <deadline>
class Config {
    std::string path_;
    std::map<std::string, std::string> values_;
    std::vector<TaskSpec> tasks_;

    void readLine(const std::string& line);

public:
    explicit Config(std::string path);

    // Calls handleLine for every non-empty line of a text file (comments removed).
    // A ConfigError thrown for a line gets the file name and line number added.
    static void forEachLine(const std::string& path, const std::function<void(const std::string&)>& handleLine);

    double number(const std::string& key) const;
    int whole(const std::string& key) const;  // a number that must be a whole number >= 0
    const std::string& text(const std::string& key) const;
    const std::vector<TaskSpec>& tasks() const { return tasks_; }

    friend std::ostream& operator<<(std::ostream& os, const Config& config);
};
