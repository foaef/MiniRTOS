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
// Preemption needs no special code: a higher priority task that was just released
// simply wins the next pick, and the interrupted task continues later.
class Scheduler {
    std::vector<Task> tasks_;
    EventLog log_;
    int now_ = 0;
    int idleTicks_ = 0;       // ticks in which no task wanted the CPU
    int switches_ = 0;        // how many times the running task changed
    std::string running_;     // name of the task that ran in the previous tick ("" = none)

    void releaseJobs();       // Step 1: start a new job for every task that is due
    void checkDeadlines();    // Step 2: look for jobs that are late
    Task* pickNext();         // Step 3: highest-priority active task, or nullptr

public:
    Scheduler(const std::vector<Task>& tasks, int logCapacity);

    // One tick of the scheduler. `bike` is needed to do the real work of a task.
    void tick(EBike& bike);

    // Fraction of ticks in which the CPU was busy (0.0 - 1.0).
    double cpuUsage() const;

    int totalMisses() const;

    friend std::ostream& operator<<(std::ostream& os, const Scheduler& scheduler);
};
