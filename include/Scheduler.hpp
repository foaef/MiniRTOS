#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "EventLog.hpp"
#include "Task.hpp"

class EBike;

// Fixed-priority preemptive scheduler. In every tick it does 4 things:
//   1. releases the jobs whose time has come
//   2. checks if any job missed its deadline
//   3. picks the active task with the highest priority
//   4. gives that task 1 tick of CPU; if that was its last tick, the real work is done
class Scheduler {
    std::vector<Task> tasks_;
    EventLog log_;
    int now_ = 0;
    int idleTicks_ = 0;
    int switches_ = 0;
    std::string running_;

    void releaseJobs();
    void checkDeadlines();
    Task* pickNext();

public:
    Scheduler(const std::vector<Task>& tasks, int logCapacity);

    void tick(EBike& bike);

    double cpuUsage() const;

    int totalMisses() const;

    friend std::ostream& operator<<(std::ostream& os, const Scheduler& scheduler);
};
