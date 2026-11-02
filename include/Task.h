#ifndef TASK_H
#define TASK_H

#include <string>
#include <ostream>
#include <vector>
#include <functional>

enum class TaskState {
    CREATED,
    READY,
    RUNNING,
    BLOCKED,
    SUSPENDED,
    TERMINATED
};

class Task {
public:
    Task();
    Task(unsigned int taskId, const std::string& taskName, 
         unsigned int priority, unsigned int stackSize);
    
    
    unsigned int getId() const;
    const std::string& getName() const;
    unsigned int getPriority() const;
    void setPriority(unsigned int priority);
    TaskState getState() const;
    void setState(TaskState state);
    unsigned int getStackSize() const;
    unsigned long getExecutionTime() const;
    void incrementExecutionTime(unsigned long ticks);
    unsigned long getCreationTime() const;
    void setCreationTime(unsigned long time);
    
    bool isReady() const;
    bool isRunning() const;
    bool isBlocked() const;
    bool isTerminated() const;
    
    void resetExecutionTime();
    bool hasHigherPriorityThan(const Task& other) const;
    
    void executeTick();
    
    void yield();
    
    bool canPreempt(const Task& other) const;
    
private:
    void updateStateInternal();
    bool validatePriority(unsigned int priority) const;
    std::string stateToString() const;
    
    unsigned int taskId_;
    std::string taskName_;
    unsigned int priority_;
    TaskState taskState_;
    unsigned int stackSize_;
    unsigned long executionTime_;
    unsigned long creationTime_;
    unsigned int tickCount_;
};

std::ostream& operator<<(std::ostream& os, const Task& task);
std::ostream& operator<<(std::ostream& os, TaskState state);

#endif // TASK_H
