#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Task.h"
#include "TaskQueue.h"
#include "Timer.h"
#include <ostream>
#include <string>
#include <cstddef>

enum class SchedulerMode {
    COOPERATIVE,
    PREEMPTIVE_ROUND_ROBIN,
    PRIORITY_BASED
};

class Scheduler {
public:
    Scheduler();
    explicit Scheduler(SchedulerMode mode);
    Scheduler(SchedulerMode mode, unsigned int maxTasks);
    
    void addTask(const Task& task);
    void addTask(Task&& task);
    
    bool removeTask(unsigned int taskId);
    void killTask(unsigned int taskId);
    
    void start();
    void stop();
    void pause();
    void resume();
    
    void scheduleTick();
    
    unsigned long runForTicks(unsigned long numTicks);
    
    bool contextSwitch();
    
    bool isRunning() const;
    SchedulerMode getMode() const;
    unsigned int getMaxTasks() const;
    std::size_t getTaskCount() const;
    const Task* getCurrentTask() const;
    unsigned long getTickCount() const;
    
    const TaskQueue& getReadyQueue() const;
    TaskQueue& getReadyQueue();
    
    void attachTimer(Timer* timer);
    Timer* getTimer() const;
    
    void printStatistics() const;
    unsigned long getTotalContextSwitches() const;
    unsigned long getIdleTicks() const;
    
private:
    void initializeScheduler();
    void selectNextTask();
    void performContextSwitch();
    bool canSwitchTask() const;
    std::string modeToString() const;
    void updateStatistics();
    bool validateTaskAddition() const;
    
    TaskQueue readyQueue_;
    TaskQueue blockedQueue_;
    Task* currentTask_;
    SchedulerMode schedulerMode_;
    bool isRunning_;
    bool isPaused_;
    unsigned int maxTasks_;
    unsigned long tickCount_;
    unsigned long contextSwitchCount_;
    unsigned long idleTicks_;
    Timer* attachedTimer_;
};

std::ostream& operator<<(std::ostream& os, const Scheduler& scheduler);
std::ostream& operator<<(std::ostream& os, SchedulerMode mode);

#endif // SCHEDULER_H
