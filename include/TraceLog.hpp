#pragma once

#include <cstddef>
#include <iosfwd>

// Remembers which task ran in each of the last `capacity` ticks (-1 = CPU idle).
// A fixed ring buffer like this is what you would use on the Pico, where memory is
// reserved up front. It owns a raw array, so it has the copy constructor,
// operator= (copy and swap) and destructor.
class TraceLog {
    int* slots_;
    std::size_t capacity_;
    std::size_t total_ = 0;  // ticks recorded since the start

public:
    explicit TraceLog(std::size_t capacity);
    TraceLog(const TraceLog& other);
    TraceLog& operator=(TraceLog other);
    ~TraceLog();

    void record(int taskIndex);

    friend void swap(TraceLog& first, TraceLog& second) noexcept;
    friend std::ostream& operator<<(std::ostream& os, const TraceLog& log);
};
