#ifndef TRACE_H
#define TRACE_H

#include "Common.h"
#include <vector>
#include <array>
#include <ostream>

namespace ebike {

enum class TraceKind { Switch, Block, Unblock, DeadlineMiss, Overrun, WatchdogReset, Alert, Info };

class TraceEvent {
    Tick tick_{0};
    TraceKind kind_{TraceKind::Info};
    std::string subject_;

public:
    TraceEvent() = default;
    TraceEvent(Tick tick, TraceKind kind, std::string subject);
    
    Tick tick() const { return tick_; }
    TraceKind kind() const { return kind_; }
    const std::string& subject() const { return subject_; }
    const char* kindName() const;

    friend std::ostream& operator<<(std::ostream& os, const TraceEvent& event);
};

class TraceBuffer {
    static constexpr std::size_t kKindCount = static_cast<std::size_t>(TraceKind::Info) + 1;

    std::size_t capacity_;
    TraceEvent* events_;
    std::size_t head_{0};
    std::size_t count_{0};
    std::size_t total_{0};
    std::array<std::size_t, kKindCount> perKind_{};

    static std::size_t validated(std::size_t capacity);
    void swap(TraceBuffer& other) noexcept;

public:
    explicit TraceBuffer(std::size_t capacity);
    TraceBuffer(const TraceBuffer& other);
    TraceBuffer& operator=(const TraceBuffer& other);
    ~TraceBuffer();

    void record(Tick tick, TraceKind kind, std::string subject);
    
    std::size_t size() const { return count_; }
    std::size_t capacity() const { return capacity_; }
    std::size_t dropped() const { return total_ - count_; }
    std::size_t countOf(TraceKind kind) const { return perKind_[static_cast<std::size_t>(kind)]; }
    const TraceEvent& at(std::size_t index) const;
    std::vector<TraceEvent> tail(std::size_t n) const;

    friend std::ostream& operator<<(std::ostream& os, const TraceBuffer& buffer);
};

}

#endif
