#include "Scheduler.h"
#include <iostream>
#include <stdexcept>

Scheduler::Scheduler() 
    : readyQueue_("ReadyQueue"), blockedQueue_("BlockedQueue"),
      currentTask_(nullptr), schedulerMode_(SchedulerMode::PRIORITY_BASED),
      isRunning_(false), isPaused_(false), maxTasks_(32),
      tickCount_(0), contextSwitchCount_(0), idleTicks_(0),
      attachedTimer_(nullptr) {
    initializeScheduler();
}

Scheduler::Scheduler(SchedulerMode mode) 
    : readyQueue_("ReadyQueue"), blockedQueue_("BlockedQueue"),
      currentTask_(nullptr), schedulerMode_(mode),
      isRunning_(false), isPaused_(false), maxTasks_(32),
      tickCount_(0), contextSwitchCount_(0), idleTicks_(0),
      attachedTimer_(nullptr) {
    initializeScheduler();
}

Scheduler::Scheduler(SchedulerMode mode, unsigned int maxTasks) 
    : readyQueue_("ReadyQueue"), blockedQueue_("BlockedQueue"),
      currentTask_(nullptr), schedulerMode_(mode),
      isRunning_(false), isPaused_(false), maxTasks_(maxTasks),
      tickCount_(0), contextSwitchCount_(0), idleTicks_(0),
      attachedTimer_(nullptr) {
    if (maxTasks_ == 0) {
        throw std::invalid_argument("Scheduler: max tasks must be greater than 0");
    }
    initializeScheduler();
}

void Scheduler::addTask(const Task& task) {
    if (!validateTaskAddition()) {
        throw std::runtime_error("Scheduler: cannot add task - maximum task limit reached");
    }
    Task taskCopy = task;
    taskCopy.setCreationTime(tickCount_);
    if (taskCopy.getState() == TaskState::CREATED) {
        taskCopy.setState(TaskState::READY);
    }
    readyQueue_.enqueue(taskCopy);
}

void Scheduler::addTask(Task&& task) {
    if (!validateTaskAddition()) {
        throw std::runtime_error("Scheduler: cannot add task - maximum task limit reached");
    }
    task.setCreationTime(tickCount_);
    if (task.getState() == TaskState::CREATED) {
        task.setState(TaskState::READY);
    }
    readyQueue_.enqueueTask(std::move(task));
}

bool Scheduler::removeTask(unsigned int taskId) {
    if (currentTask_ && currentTask_->getId() == taskId) {
        currentTask_ = nullptr;
    }
    TaskQueue newQueue("ReadyQueue");
    for (std::size_t i = 0; i < readyQueue_.getSize(); ++i) {
        try {
            const Task& t = readyQueue_.getTaskAt(i);
            if (t.getId() != taskId) {
                newQueue.enqueue(t);
            }
        } catch (const std::out_of_range&) {
            break;
        }
    }
    return true;
}

void Scheduler::killTask(unsigned int taskId) {
    if (currentTask_ && currentTask_->getId() == taskId) {
        currentTask_->setState(TaskState::TERMINATED);
        currentTask_ = nullptr;
    }
    for (std::size_t i = 0; i < readyQueue_.getSize(); ++i) {
        try {
            Task& t = readyQueue_.getTaskAt(i);
            if (t.getId() == taskId) {
                t.setState(TaskState::TERMINATED);
                break;
            }
        } catch (const std::out_of_range&) {
            break;
        }
    }
    readyQueue_.removeTerminatedTasks();
}

void Scheduler::start() {
    if (!isRunning_) {
        isRunning_ = true;
        isPaused_ = false;
        if (attachedTimer_) {
            attachedTimer_->start();
        }
    }
}

void Scheduler::stop() {
    isRunning_ = false;
    isPaused_ = false;
    if (attachedTimer_) {
        attachedTimer_->stop();
    }
    currentTask_ = nullptr;
}

void Scheduler::pause() {
    if (isRunning_) {
        isPaused_ = true;
    }
}

void Scheduler::resume() {
    if (isRunning_) {
        isPaused_ = false;
    }
}

void Scheduler::scheduleTick() {
    if (!isRunning_ || isPaused_) {
        if (isRunning_) {
            idleTicks_++;
        }
        return;
    }
    
    tickCount_++;
    
    if (attachedTimer_) {
        attachedTimer_->tick();
    }
    
    readyQueue_.removeTerminatedTasks();
    
    selectNextTask();
    
    if (currentTask_) {
        currentTask_->executeTick();
        updateStatistics();
    } else {
        idleTicks_++;
    }
}

unsigned long Scheduler::runForTicks(unsigned long numTicks) {
    unsigned long ticksRun = 0;
    bool wasRunning = isRunning_;
    if (!wasRunning) {
        start();
    }
    
    for (unsigned long i = 0; i < numTicks; ++i) {
        if (readyQueue_.isEmpty() && !currentTask_) {
            break;
        }
        scheduleTick();
        ticksRun++;
    }
    
    if (!wasRunning) {
        stop();
    }
    
    return ticksRun;
}

bool Scheduler::contextSwitch() {
    if (!canSwitchTask()) {
        return false;
    }
    performContextSwitch();
    return true;
}

bool Scheduler::isRunning() const {
    return isRunning_;
}

SchedulerMode Scheduler::getMode() const {
    return schedulerMode_;
}

unsigned int Scheduler::getMaxTasks() const {
    return maxTasks_;
}

std::size_t Scheduler::getTaskCount() const {
    return readyQueue_.getSize() + (currentTask_ ? 1 : 0);
}

const Task* Scheduler::getCurrentTask() const {
    return currentTask_;
}

unsigned long Scheduler::getTickCount() const {
    return tickCount_;
}

const TaskQueue& Scheduler::getReadyQueue() const {
    return readyQueue_;
}

TaskQueue& Scheduler::getReadyQueue() {
    return readyQueue_;
}

void Scheduler::attachTimer(Timer* timer) {
    attachedTimer_ = timer;
}

Timer* Scheduler::getTimer() const {
    return attachedTimer_;
}

void Scheduler::printStatistics() const {
    std::cout << "\n=== Scheduler Statistics ===\n";
    std::cout << "Mode: " << modeToString() << "\n";
    std::cout << "Running: " << (isRunning_ ? "Yes" : "No") << "\n";
    std::cout << "Total ticks: " << tickCount_ << "\n";
    std::cout << "Context switches: " << contextSwitchCount_ << "\n";
    std::cout << "Idle ticks: " << idleTicks_ << "\n";
    std::cout << "Task count: " << getTaskCount() << "\n";
    std::cout << "===========================\n\n";
}

unsigned long Scheduler::getTotalContextSwitches() const {
    return contextSwitchCount_;
}

unsigned long Scheduler::getIdleTicks() const {
    return idleTicks_;
}

void Scheduler::initializeScheduler() {
    currentTask_ = nullptr;
}

void Scheduler::selectNextTask() {
    if (readyQueue_.isEmpty()) {
        if (currentTask_ && currentTask_->isRunning()) {
            currentTask_->yield();
        }
        return;
    }
    
    readyQueue_.sortByPriority();
    
    if (schedulerMode_ == SchedulerMode::PRIORITY_BASED) {
        if (!currentTask_ || currentTask_->getState() == TaskState::TERMINATED) {
            if (!readyQueue_.isEmpty()) {
                try {
                    Task nextTask = readyQueue_.dequeue();
                    nextTask.setState(TaskState::RUNNING);
                    static Task storedTask;
                    storedTask = nextTask;
                    currentTask_ = &storedTask;
                    contextSwitchCount_++;
                } catch (const std::runtime_error&) {
                    currentTask_ = nullptr;
                }
            }
        } else if (currentTask_->isRunning()) {
            try {
                Task highest = readyQueue_.peek();
                if (highest.getPriority() > currentTask_->getPriority()) {
                    contextSwitch();
                }
            } catch (const std::runtime_error&) {
            }
        }
    } else if (schedulerMode_ == SchedulerMode::PREEMPTIVE_ROUND_ROBIN) {
        if (currentTask_ && currentTask_->isRunning()) {
            currentTask_->yield();
            if (currentTask_->getState() != TaskState::TERMINATED) {
                currentTask_->setState(TaskState::READY);
                readyQueue_.enqueue(*currentTask_);
            }
            currentTask_ = nullptr;
        }
        if (!readyQueue_.isEmpty() && !currentTask_) {
            try {
                Task nextTask = readyQueue_.dequeue();
                nextTask.setState(TaskState::RUNNING);
                static Task storedTask2;
                storedTask2 = nextTask;
                currentTask_ = &storedTask2;
                contextSwitchCount_++;
            } catch (const std::runtime_error&) {
                currentTask_ = nullptr;
            }
        }
    }
}

void Scheduler::performContextSwitch() {
    if (currentTask_ && currentTask_->isRunning()) {
        currentTask_->yield();
        if (currentTask_->getState() != TaskState::TERMINATED) {
            currentTask_->setState(TaskState::READY);
            readyQueue_.enqueue(*currentTask_);
        }
    }
    
    if (!readyQueue_.isEmpty()) {
        try {
            Task nextTask = readyQueue_.dequeue();
            nextTask.setState(TaskState::RUNNING);
            static Task storedTask3;
            storedTask3 = nextTask;
            currentTask_ = &storedTask3;
            contextSwitchCount_++;
        } catch (const std::runtime_error&) {
            currentTask_ = nullptr;
        }
    } else {
        currentTask_ = nullptr;
    }
}

bool Scheduler::canSwitchTask() const {
    return isRunning_ && !isPaused_ && !readyQueue_.isEmpty();
}

std::string Scheduler::modeToString() const {
    switch (schedulerMode_) {
        case SchedulerMode::COOPERATIVE: return "COOPERATIVE";
        case SchedulerMode::PREEMPTIVE_ROUND_ROBIN: return "PREEMPTIVE_ROUND_ROBIN";
        case SchedulerMode::PRIORITY_BASED: return "PRIORITY_BASED";
        default: return "UNKNOWN";
    }
}

void Scheduler::updateStatistics() {
}

bool Scheduler::validateTaskAddition() const {
    return getTaskCount() < maxTasks_;
}

std::ostream& operator<<(std::ostream& os, SchedulerMode mode) {
    switch (mode) {
        case SchedulerMode::COOPERATIVE: os << "COOPERATIVE"; break;
        case SchedulerMode::PREEMPTIVE_ROUND_ROBIN: os << "PREEMPTIVE_ROUND_ROBIN"; break;
        case SchedulerMode::PRIORITY_BASED: os << "PRIORITY_BASED"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Scheduler& scheduler) {
    os << "Scheduler[mode=" << scheduler.getMode()
       << ", running=" << (scheduler.isRunning() ? "true" : "false")
       << ", tasks=" << scheduler.getTaskCount()
       << ", ticks=" << scheduler.getTickCount()
       << ", contextSwitches=" << scheduler.getTotalContextSwitches() << "]\n";
    if (scheduler.getCurrentTask()) {
        os << "  Current: " << *scheduler.getCurrentTask() << "\n";
    } else {
        os << "  Current: None (IDLE)\n";
    }
    os << scheduler.getReadyQueue();
    return os;
}
