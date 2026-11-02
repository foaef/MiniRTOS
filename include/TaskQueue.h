#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include "Task.h"
#include <vector>
#include <ostream>
#include <cstddef>

class TaskQueue {
public:
    TaskQueue();
    explicit TaskQueue(const std::string& queueName);
    
    void enqueue(const Task& task);
    void enqueueTask(Task&& task);
    
    Task dequeue();
    Task peek() const;
    
    bool isEmpty() const;
    std::size_t getSize() const;
    void clear();
    const std::string& getQueueName() const;
    
    void sortByPriority();
    
    unsigned int removeTerminatedTasks();
    
    bool findAndPromoteTask(unsigned int taskId, unsigned int newPriority);
    
    Task& getTaskAt(std::size_t index);
    const Task& getTaskAt(std::size_t index) const;
    
    bool containsTask(unsigned int taskId) const;
    
private:
    void sortTasksInternal();
    bool comparePriority(const Task& a, const Task& b) const;
    void validateQueueState() const;
    
    std::vector<Task> tasks_;
    std::string queueName_;
    unsigned long enqueueCount_;
};

std::ostream& operator<<(std::ostream& os, const TaskQueue& queue);

#endif // TASKQUEUE_H
