#include "TraceLog.hpp"

#include <algorithm>
#include <ostream>
#include <utility>

#include "Errors.hpp"

TraceLog::TraceLog(std::size_t capacity) : slots_(nullptr), capacity_(capacity) {
    if (capacity_ == 0) {
        throw ConfigError("trace capacity must be greater than 0");
    }
    slots_ = new int[capacity_]();
}

TraceLog::TraceLog(const TraceLog& other)
    : slots_(new int[other.capacity_]), capacity_(other.capacity_), total_(other.total_) {
    std::copy(other.slots_, other.slots_ + other.capacity_, slots_);
}

// `other` is already a copy (taken by value); swapping hands our old array to it,
// and it is freed when `other` goes out of scope.
TraceLog& TraceLog::operator=(TraceLog other) {
    swap(*this, other);
    return *this;
}

TraceLog::~TraceLog() { delete[] slots_; }

void TraceLog::record(int taskIndex) {
    slots_[total_ % capacity_] = taskIndex;
    total_++;
}

void swap(TraceLog& first, TraceLog& second) noexcept {
    using std::swap;
    swap(first.slots_, second.slots_);
    swap(first.capacity_, second.capacity_);
    swap(first.total_, second.total_);
}

std::ostream& operator<<(std::ostream& os, const TraceLog& log) {
    const std::size_t first = log.total_ > log.capacity_ ? log.total_ - log.capacity_ : 0;
    os << "ticks " << first << ".." << log.total_ << ":";
    for (std::size_t i = first; i < log.total_; i++) {
        const int value = log.slots_[i % log.capacity_];
        if (value < 0) {
            os << " .";
        } else {
            os << ' ' << value;
        }
    }
    return os;
}
