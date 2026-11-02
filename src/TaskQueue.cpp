#include "TaskQueue.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>

TaskQueue::TaskQueue() 
    : queueName_("default"), enqueueCount_(0) {
}

TaskQueue::TaskQueue(const std::string& queueName) 
    : queueName_(queueName), enqueueCount_(0) {
    if (queueName_.empty()) {
        throw std::invalid_argument("TaskQueue: queue name cannot be empty");
    }
}

void TaskQueue::enqueue(const Task& task) {
    Task taskCopy = task;
    if (taskCopy.getState() == TaskState::CREATED) {
        taskCopy.setState(TaskState::READY);
    }
    tasks_.push_back(taskCopy);
    enqueueCount_++;
    validateQueueState();
}

void TaskQueue::enqueueTask(Task&& task) {
    if (task.getState() == TaskState::CREATED) {
        task.setState(TaskState::READY);
    }
    tasks_.push_back(std::move(task));
    enqueueCount_++;
    validateQueueState();
}

Task TaskQueue::dequeue() {
    if (isEmpty()) {
        throw std::runtime_error("TaskQueue: cannot dequeue from empty queue");
    }
    sortByPriority();
    Task frontTask = tasks_.front();
    tasks_.erase(tasks_.begin());
    return frontTask;
}

Task TaskQueue::peek() const {
    if (isEmpty()) {
        throw std::runtime_error("TaskQueue: cannot peek empty queue");
    }
    Task highestPriority = tasks_[0];
    for (const auto& task : tasks_) {
        if (task.hasHigherPriorityThan(highestPriority)) {
            highestPriority = task;
        }
    }
    return highestPriority;
}

bool TaskQueue::isEmpty() const {
    return tasks_.empty();
}

std::size_t TaskQueue::getSize() const {
    return tasks_.size();
}

void TaskQueue::clear() {
    tasks_.clear();
}

const std::string& TaskQueue::getQueueName() const {
    return queueName_;
}

void TaskQueue::sortByPriority() {
    sortTasksInternal();
}

unsigned int TaskQueue::removeTerminatedTasks() {
    std::size_t originalSize = tasks_.size();
    tasks_.erase(
        std::remove_if(tasks_.begin(), tasks_.end(),
            [](const Task& task) {
                return task.getState() == TaskState::TERMINATED;
            }),
        tasks_.end()
    );
    return static_cast<unsigned int>(originalSize - tasks_.size());
}

bool TaskQueue::findAndPromoteTask(unsigned int taskId, unsigned int newPriority) {
    for (auto& task : tasks_) {
        if (task.getId() == taskId) {
            try {
                if (newPriority > task.getPriority()) {
                    task.setPriority(newPriority);
                    sortTasksInternal();
                    return true;
                }
            } catch (const std::invalid_argument&) {
                return false;
            }
        }
    }
    return false;
}

Task& TaskQueue::getTaskAt(std::size_t index) {
    if (index >= tasks_.size()) {
        throw std::out_of_range("TaskQueue: index out of range");
    }
    return tasks_[index];
}

const Task& TaskQueue::getTaskAt(std::size_t index) const {
    if (index >= tasks_.size()) {
        throw std::out_of_range("TaskQueue: index out of range");
    }
    return tasks_[index];
}

bool TaskQueue::containsTask(unsigned int taskId) const {
    for (const auto& task : tasks_) {
        if (task.getId() == taskId) {
            return true;
        }
    }
    return false;
}

void TaskQueue::sortTasksInternal() {
    std::sort(tasks_.begin(), tasks_.end(),
        [this](const Task& a, const Task& b) {
            return comparePriority(a, b);
        });
}

bool TaskQueue::comparePriority(const Task& a, const Task& b) const {
    if (a.getPriority() != b.getPriority()) {
        return a.getPriority() > b.getPriority();
    }
    return a.getCreationTime() < b.getCreationTime();
}

void TaskQueue::validateQueueState() const {
    if (enqueueCount_ < tasks_.size()) {
    }
}

std::ostream& operator<<(std::ostream& os, const TaskQueue& queue) {
    os << "TaskQueue[name=" << queue.getQueueName() 
       << ", size=" << queue.getSize() << "]\n";
    if (queue.isEmpty()) {
        os << "  (empty)\n";
    } else {
        for (std::size_t i = 0; i < queue.getSize(); ++i) {
            try {
                os << "  [" << i << "] " << queue.getTaskAt(i) << "\n";
            } catch (const std::out_of_range&) {
                break;
            }
        }
    }
    return os;
}
