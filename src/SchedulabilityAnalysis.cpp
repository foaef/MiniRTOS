#include "SchedulabilityAnalysis.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <ostream>

SchedulabilityAnalysis::SchedulabilityAnalysis(const std::vector<Task>& tasks) : tasks_(tasks) {
    std::stable_sort(tasks_.begin(), tasks_.end(),
                     [](const Task& a, const Task& b) { return a.priority() > b.priority(); });
}

int SchedulabilityAnalysis::interference(std::size_t index, int window) const {
    int stolen = 0;
    for (std::size_t other = 0; other < tasks_.size(); other++) {
        if (other == index || tasks_[other].priority() < tasks_[index].priority()) {
            continue;
        }
        const int jobs = (window + tasks_[other].period() - 1) / tasks_[other].period();  // rounded up
        stolen = stolen + jobs * tasks_[other].wcet();
    }
    return stolen;
}

double SchedulabilityAnalysis::utilization() const {
    return std::accumulate(tasks_.begin(), tasks_.end(), 0.0, [](double total, const Task& task) {
        return total + static_cast<double>(task.wcet()) / task.period();
    });
}

double SchedulabilityAnalysis::utilizationBound() const {
    if (tasks_.empty()) {
        return 1.0;
    }
    const double count = static_cast<double>(tasks_.size());
    return count * (std::pow(2.0, 1.0 / count) - 1.0);
}

// Start with the wcet and add the interference until the answer stops changing
// (every round can only grow the answer, so it either settles or passes the deadline).
int SchedulabilityAnalysis::worstResponse(std::size_t index) const {
    const Task& task = tasks_.at(index);
    int response = task.wcet();
    while (response <= task.deadline()) {
        const int next = task.wcet() + interference(index, response);
        if (next == response) {
            break;
        }
        response = next;
    }
    return response;
}

bool SchedulabilityAnalysis::meetsDeadline(std::size_t index) const {
    return worstResponse(index) <= tasks_.at(index).deadline();
}

bool SchedulabilityAnalysis::isSchedulable() const {
    for (std::size_t index = 0; index < tasks_.size(); index++) {
        if (!meetsDeadline(index)) {
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& os, const SchedulabilityAnalysis& analysis) {
    os << "SchedulabilityAnalysis{" << std::fixed << std::setprecision(1) << analysis.utilization() * 100
       << "% of the CPU needed, rate-monotonic bound " << analysis.utilizationBound() * 100
       << "%, schedulable: " << (analysis.isSchedulable() ? "yes" : "NO") << "}";
    for (std::size_t index = 0; index < analysis.tasks_.size(); index++) {
        const Task& task = analysis.tasks_[index];
        const bool ok = analysis.meetsDeadline(index);
        os << "\n  " << task.name() << ": worst response ";
        if (ok) {
            os << analysis.worstResponse(index);
        } else {
            os << "over " << task.deadline();
        }
        os << " (deadline " << task.deadline() << ", " << (ok ? "ok" : "MISS") << "), simulation saw ";
        if (task.worstResponse() == 0) {
            os << "no finished job";
        } else {
            os << task.worstResponse();
        }
    }
    return os;
}
