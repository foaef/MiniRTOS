#include "Timer.h"
#include <stdexcept>
#include <algorithm>

Timer::Timer() 
    : currentTick_(0), tickInterval_(1), isRunning_(false), 
      startTime_(0), callbackCount_(0) {
    pendingCallbacks_ = new std::vector<unsigned long>();
    initializeCallbacks();
}

Timer::Timer(unsigned long tickInterval) 
    : currentTick_(0), tickInterval_(tickInterval), isRunning_(false),
      startTime_(0), callbackCount_(0) {
    if (!isValidTickInterval(tickInterval)) {
        throw std::invalid_argument("Timer: tick interval must be greater than 0");
    }
    pendingCallbacks_ = new std::vector<unsigned long>();
    initializeCallbacks();
}


Timer::Timer(const Timer& other) 
    : currentTick_(other.currentTick_), tickInterval_(other.tickInterval_),
      isRunning_(other.isRunning_), startTime_(other.startTime_),
      callbackCount_(other.callbackCount_) {
    pendingCallbacks_ = new std::vector<unsigned long>();
    copyCallbacks(other);
}

Timer& Timer::operator=(const Timer& other) {
    if (this == &other) {
        return *this;
    }
    currentTick_ = other.currentTick_;
    tickInterval_ = other.tickInterval_;
    isRunning_ = other.isRunning_;
    startTime_ = other.startTime_;
    callbackCount_ = other.callbackCount_;
    copyCallbacks(other);
    return *this;
}

Timer::~Timer() {
    clearCallbacks();
    delete pendingCallbacks_;
    pendingCallbacks_ = nullptr;
}

void Timer::start() {
    if (!isRunning_) {
        isRunning_ = true;
        if (startTime_ == 0) {
            startTime_ = currentTick_;
        }
    }
}

void Timer::stop() {
    isRunning_ = false;
}

void Timer::reset() {
    currentTick_ = 0;
    startTime_ = 0;
    clearCallbacks();
    callbackCount_ = 0;
    initializeCallbacks();
}

void Timer::tick() {
    if (isRunning_) {
        currentTick_ += tickInterval_;
        processCallbacks();
    }
}

unsigned long Timer::getCurrentTick() const {
    return currentTick_;
}

unsigned long Timer::getTickInterval() const {
    return tickInterval_;
}

void Timer::setTickInterval(unsigned long interval) {
    if (!isValidTickInterval(interval)) {
        throw std::invalid_argument("Timer: tick interval must be greater than 0");
    }
    tickInterval_ = interval;
}

bool Timer::isRunning() const {
    return isRunning_;
}

unsigned long Timer::getUptime() const {
    if (startTime_ == 0) {
        return 0;
    }
    return currentTick_ >= startTime_ ? (currentTick_ - startTime_) : 0;
}

void Timer::scheduleCallback(unsigned long targetTick) {
    if (targetTick < currentTick_) {
        throw std::invalid_argument("Timer: target tick cannot be in the past");
    }
    pendingCallbacks_->push_back(targetTick);
    callbackCount_++;
    std::sort(pendingCallbacks_->begin(), pendingCallbacks_->end());
}

unsigned int Timer::processCallbacks() {
    unsigned int executedCount = 0;
    auto it = pendingCallbacks_->begin();
    while (it != pendingCallbacks_->end()) {
        if (*it <= currentTick_) {
            executedCount++;
            it = pendingCallbacks_->erase(it);
        } else {
            ++it;
        }
    }
    return executedCount;
}

unsigned long Timer::timeUntilNextCallback() const {
    if (pendingCallbacks_->empty()) {
        return 0;
    }
    unsigned long nextTick = *std::min_element(pendingCallbacks_->begin(), pendingCallbacks_->end());
    if (nextTick <= currentTick_) {
        return 0;
    }
    return nextTick - currentTick_;
}

bool Timer::hasPendingCallbacks() const {
    return !pendingCallbacks_->empty();
}

std::size_t Timer::getPendingCallbackCount() const {
    return pendingCallbacks_->size();
}

void Timer::initializeCallbacks() {
    if (pendingCallbacks_->empty()) {
    }
}

void Timer::copyCallbacks(const Timer& other) {
    clearCallbacks();
    if (other.pendingCallbacks_) {
        *pendingCallbacks_ = *other.pendingCallbacks_;
    }
    callbackCount_ = other.callbackCount_;
}

void Timer::clearCallbacks() {
    if (pendingCallbacks_) {
        pendingCallbacks_->clear();
    }
}

bool Timer::isValidTickInterval(unsigned long interval) const {
    return interval > 0;
}

std::ostream& operator<<(std::ostream& os, const Timer& timer) {
    os << "Timer[currentTick=" << timer.getCurrentTick()
       << ", interval=" << timer.getTickInterval()
       << ", running=" << (timer.isRunning() ? "true" : "false")
       << ", uptime=" << timer.getUptime()
       << ", pendingCallbacks=" << timer.getPendingCallbackCount() << "]";
    return os;
}
