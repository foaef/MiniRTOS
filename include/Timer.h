#ifndef TIMER_H
#define TIMER_H

#include <ostream>
#include <vector>
#include <cstddef>

class Scheduler;

class Timer {
public:
    Timer();
    explicit Timer(unsigned long tickInterval);
    
    Timer(const Timer& other);
    Timer& operator=(const Timer& other);
    ~Timer();
    
    void start();
    void stop();
    void reset();
    void tick();
    
    unsigned long getCurrentTick() const;
    unsigned long getTickInterval() const;
    void setTickInterval(unsigned long interval);
    
    bool isRunning() const;
    unsigned long getUptime() const;
    
    void scheduleCallback(unsigned long targetTick);
    
    unsigned int processCallbacks();
    
    unsigned long timeUntilNextCallback() const;
    
    bool hasPendingCallbacks() const;
    std::size_t getPendingCallbackCount() const;
    
private:
    void initializeCallbacks();
    void copyCallbacks(const Timer& other);
    void clearCallbacks();
    bool isValidTickInterval(unsigned long interval) const;
    
    unsigned long currentTick_;
    unsigned long tickInterval_;
    bool isRunning_;
    unsigned long startTime_;
    std::vector<unsigned long>* pendingCallbacks_;
    unsigned long callbackCount_;
};

std::ostream& operator<<(std::ostream& os, const Timer& timer);

#endif // TIMER_H
