#pragma once

#include <cstddef>
#include <iosfwd>
#include <vector>

#include "Task.hpp"

// Checks on paper whether every task meets its deadline. It assumes fixed priorities,
// deadline <= period, and the worst case: all tasks release a job at tick 0.
//   - CPU share: the sum of wcet / period. Below the rate-monotonic bound
//     n * (2^(1/n) - 1) every task set is schedulable.
//   - Response time: a job's own wcet plus the time taken by the jobs of all other
//     tasks with the same or a higher priority (the safe side) released before it ends.
// Tasks are sorted by priority, highest first, and `index` counts in that order.
class SchedulabilityAnalysis {
    std::vector<Task> tasks_;

    // CPU time taken from task `index` by other jobs released during `window` ticks.
    int interference(std::size_t index, int window) const;

public:
    explicit SchedulabilityAnalysis(const std::vector<Task>& tasks);

    double utilization() const;

    double utilizationBound() const;

    // Past the deadline the search stops, so a result over the deadline is only a lower bound.
    int worstResponse(std::size_t index) const;

    bool meetsDeadline(std::size_t index) const;

    bool isSchedulable() const;

    friend std::ostream& operator<<(std::ostream& os, const SchedulabilityAnalysis& analysis);
};
