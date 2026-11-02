#include "EventLog.hpp"

#include <ostream>

#include "ConfigError.hpp"

std::string* EventLog::copyLines(const EventLog& other) {
    std::string* copy = new std::string[other.capacity_];
    for (int i = 0; i < other.count_; i++) {
        copy[i] = other.lines_[i];
    }
    return copy;
}

EventLog::EventLog(int capacity) : capacity_(capacity), lines_(nullptr) {
    if (capacity_ < 0) {
        throw ConfigError("event log capacity cannot be negative");
    }
    lines_ = new std::string[capacity_];
}

EventLog::EventLog(const EventLog& other)
    : capacity_(other.capacity_), lines_(copyLines(other)), count_(other.count_), dropped_(other.dropped_) {}

EventLog& EventLog::operator=(const EventLog& other) {
    if (this == &other) {
        return *this;
    }
    std::string* newLines = copyLines(other);
    delete[] lines_;
    lines_ = newLines;
    capacity_ = other.capacity_;
    count_ = other.count_;
    dropped_ = other.dropped_;
    return *this;
}

EventLog::~EventLog() { delete[] lines_; }

void EventLog::add(const std::string& text) {
    if (count_ == capacity_) {
        dropped_++;
        return;
    }
    lines_[count_] = text;
    count_++;
}

std::ostream& operator<<(std::ostream& os, const EventLog& log) {
    os << "EventLog{" << log.count_ << "/" << log.capacity_ << " stored, " << log.dropped_ << " dropped}";
    for (int i = 0; i < log.count_; i++) {
        os << "\n  " << log.lines_[i];
    }
    return os;
}
