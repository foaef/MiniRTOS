#include "Task.h"
#include <iostream>
#include <stdexcept>

Task::Task() 
    : taskId_(0), taskName_("idle"), priority_(0), 
      taskState_(TaskState::CREATED), stackSize_(256),
      executionTime_(0), creationTime_(0), tickCount_(0) {
}

Task::Task(unsigned int taskId, const std::string& taskName, 
           unsigned int priority, unsigned int stackSize)
    : taskId_(taskId), taskName_(taskName), priority_(priority),
      taskState_(TaskState::CREATED), stackSize_(stackSize),
      executionTime_(0), creationTime_(0), tickCount_(0) {
    if (!validatePriority(priority)) {
        throw std::invalid_argument("Task: priority must be between 0 and 255");
    }
    if (taskName_.empty()) {
        throw std::invalid_argument("Task: task name cannot be empty");
    }
    if (stackSize_ < 64) {
        throw std::invalid_argument("Task: stack size must be at least 64 bytes");
    }
}

unsigned int Task::getId() const {
    return taskId_;
}

const std::string& Task::getName() const {
    return taskName_;
}

unsigned int Task::getPriority() const {
    return priority_;
}

void Task::setPriority(unsigned int priority) {
    if (!validatePriority(priority)) {
        throw std::invalid_argument("Task: priority must be between 0 and 255");
    }
    priority_ = priority;
}

TaskState Task::getState() const {
    return taskState_;
}

void Task::setState(TaskState state) {
    taskState_ = state;
    updateStateInternal();
}

unsigned int Task::getStackSize() const {
    return stackSize_;
}

unsigned long Task::getExecutionTime() const {
    return executionTime_;
}

void Task::incrementExecutionTime(unsigned long ticks) {
    if (ticks == 0) {
        return;
    }
    executionTime_ += ticks;
    tickCount_ += static_cast<unsigned int>(ticks % 1000);
    updateStateInternal();
}

unsigned long Task::getCreationTime() const {
    return creationTime_;
}

void Task::setCreationTime(unsigned long time) {
    creationTime_ = time;
}

bool Task::isReady() const {
    return taskState_ == TaskState::READY;
}

bool Task::isRunning() const {
    return taskState_ == TaskState::RUNNING;
}

bool Task::isBlocked() const {
    return taskState_ == TaskState::BLOCKED;
}

bool Task::isTerminated() const {
    return taskState_ == TaskState::TERMINATED;
}

void Task::resetExecutionTime() {
    executionTime_ = 0;
    tickCount_ = 0;
}

bool Task::hasHigherPriorityThan(const Task& other) const {
    return priority_ > other.priority_;
}

void Task::executeTick() {
    if (taskState_ == TaskState::READY || taskState_ == TaskState::RUNNING) {
        incrementExecutionTime(1);
        if (taskState_ == TaskState::CREATED) {
            taskState_ = TaskState::READY;
        } else if (taskState_ == TaskState::READY) {
            taskState_ = TaskState::RUNNING;
        }
    }
}

void Task::yield() {
    if (taskState_ == TaskState::RUNNING) {
        taskState_ = TaskState::READY;
    }
}

bool Task::canPreempt(const Task& other) const {
    if (taskState_ != TaskState::RUNNING) {
        return false;
    }
    if (other.taskState_ != TaskState::READY) {
        return false;
    }
    return hasHigherPriorityThan(other);
}

void Task::updateStateInternal() {
    if (executionTime_ > 1000000 && taskState_ != TaskState::TERMINATED) {
    }
}

bool Task::validatePriority(unsigned int priority) const {
    return priority <= 255;
}

std::string Task::stateToString() const {
    switch (taskState_) {
        case TaskState::CREATED: return "CREATED";
        case TaskState::READY: return "READY";
        case TaskState::RUNNING: return "RUNNING";
        case TaskState::BLOCKED: return "BLOCKED";
        case TaskState::SUSPENDED: return "SUSPENDED";
        case TaskState::TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

std::ostream& operator<<(std::ostream& os, TaskState state) {
    switch (state) {
        case TaskState::CREATED: os << "CREATED"; break;
        case TaskState::READY: os << "READY"; break;
        case TaskState::RUNNING: os << "RUNNING"; break;
        case TaskState::BLOCKED: os << "BLOCKED"; break;
        case TaskState::SUSPENDED: os << "SUSPENDED"; break;
        case TaskState::TERMINATED: os << "TERMINATED"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Task& task) {
    os << "Task[id=" << task.getId() 
       << ", name=" << task.getName()
       << ", priority=" << task.getPriority()
       << ", state=" << task.getState()
       << ", stack=" << task.getStackSize() << "bytes"
       << ", execTime=" << task.getExecutionTime() << "ticks]";
    return os;
}
