#include "Scheduler.hpp"

#include <iomanip>
#include <numeric>
#include <ostream>

#include "EBike.hpp"

Scheduler::Scheduler(const std::vector<Task>& tasks, int logCapacity) : tasks_(tasks), log_(logCapacity) {}

void Scheduler::releaseJobs() {
    for (Task& task : tasks_) {
        if (!task.isDue(now_)) {
            continue;
        }
        if (task.isActive()) {
            // the previous job is still not finished
            task.skipRelease();
            log_.add("t=" + std::to_string(now_) + " overrun: " + task.name());
        } else {
            task.release(now_);
        }
    }
}

void Scheduler::checkDeadlines() {
    for (Task& task : tasks_) {
        if (task.checkDeadline(now_)) {
            log_.add("t=" + std::to_string(now_) + " deadline missed: " + task.name());
        }
    }
}

Task* Scheduler::pickNext() {
    Task* best = nullptr;
    for (Task& task : tasks_) {
        if (!task.isActive()) {
            continue;
        }
        if (best == nullptr || task.priority() > best->priority()) {
            best = &task;
        }
    }
    return best;
}

void Scheduler::tick(EBike& bike) {
    releaseJobs();
    checkDeadlines();

    Task* next = pickNext();
    if (next == nullptr) {
        idleTicks_++;
        running_ = "";
    } else {
        if (next->name() != running_) {
            switches_++;
        }
        running_ = next->name();

        // Step 4: one tick of CPU for the chosen task.
        next->runOneTick();
        if (next->isFinished()) {
            bike.runJob(next->type());
            next->finish();
        }
    }
    now_++;
}

double Scheduler::cpuUsage() const {
    if (now_ == 0) {
        return 0;
    }
    return static_cast<double>(now_ - idleTicks_) / now_;
}

int Scheduler::totalMisses() const {
    return std::accumulate(tasks_.begin(), tasks_.end(), 0,
                           [](int total, const Task& task) { return total + task.misses(); });
}

std::ostream& operator<<(std::ostream& os, const Scheduler& scheduler) {
    os << "Scheduler{" << scheduler.now_ << " ticks, CPU " << std::fixed << std::setprecision(1)
       << scheduler.cpuUsage() * 100 << "%, " << scheduler.switches_ << " context switches}\n";
    for (const Task& task : scheduler.tasks_) {
        os << "  " << task << "\n";
    }
    os << scheduler.log_;
    return os;
}
