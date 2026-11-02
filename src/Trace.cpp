#include "Trace.h"
#include <algorithm>
#include <stdexcept>

namespace ebike {

TraceEvent::TraceEvent(Tick tick, TraceKind kind, std::string subject)
    : tick_(tick), kind_(kind), subject_(std::move(subject)) {}

const char* TraceEvent::kindName() const {
    switch (kind_) {
        case TraceKind::Switch: return "switch";
        case TraceKind::Block: return "block";
        case TraceKind::Unblock: return "unblock";
        case TraceKind::DeadlineMiss: return "deadline-miss";
        case TraceKind::Overrun: return "overrun";
        case TraceKind::WatchdogReset: return "watchdog";
        case TraceKind::Alert: return "alert";
        case TraceKind::Info: return "info";
    }
    return "unknown";
}

std::ostream& operator<<(std::ostream& os, const TraceEvent& event) {
    return os << "[" << std::setw(6) << event.tick_ << "] " << event.kindName() << ": " << event.subject_;
}

std::size_t TraceBuffer::validated(std::size_t capacity) {
    if (capacity == 0) {
        throw ConfigError("trace capacity must be positive");
    }
    return capacity;
}

void TraceBuffer::swap(TraceBuffer& other) noexcept {
    std::swap(capacity_, other.capacity_);
    std::swap(events_, other.events_);
    std::swap(head_, other.head_);
    std::swap(count_, other.count_);
    std::swap(total_, other.total_);
    std::swap(perKind_, other.perKind_);
}

TraceBuffer::TraceBuffer(std::size_t capacity) : capacity_(validated(capacity)), events_(new TraceEvent[capacity_]) {}

TraceBuffer::TraceBuffer(const TraceBuffer& other)
    : capacity_(other.capacity_),
      events_(new TraceEvent[other.capacity_]),
      head_(other.head_),
      count_(other.count_),
      total_(other.total_),
      perKind_(other.perKind_) {
    std::copy(other.events_, other.events_ + other.capacity_, events_);
}

TraceBuffer& TraceBuffer::operator=(const TraceBuffer& other) {
    TraceBuffer copy(other);
    swap(copy);
    return *this;
}

TraceBuffer::~TraceBuffer() { delete[] events_; }

void TraceBuffer::record(Tick tick, TraceKind kind, std::string subject) {
    events_[head_] = TraceEvent(tick, kind, std::move(subject));
    head_ = (head_ + 1) % capacity_;
    count_ = std::min(count_ + 1, capacity_);
    ++total_;
    ++perKind_[static_cast<std::size_t>(kind)];
}

const TraceEvent& TraceBuffer::at(std::size_t index) const {
    if (index >= count_) {
        throw std::out_of_range("trace index out of range");
    }
    const std::size_t oldest = (head_ + capacity_ - count_) % capacity_;
    return events_[(oldest + index) % capacity_];
}

std::vector<TraceEvent> TraceBuffer::tail(std::size_t n) const {
    const std::size_t wanted = std::min(n, count_);
    std::vector<TraceEvent> result;
    result.reserve(wanted);
    for (std::size_t i = count_ - wanted; i < count_; ++i) {
        result.push_back(at(i));
    }
    return result;
}

std::ostream& operator<<(std::ostream& os, const TraceBuffer& buffer) {
    return os << "TraceBuffer{stored=" << buffer.count_ << "/" << buffer.capacity_
              << ", dropped=" << buffer.dropped() << "}";
}

}
