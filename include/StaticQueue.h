#ifndef STATICQUEUE_H
#define STATICQUEUE_H

#include <array>
#include <cstddef>
#include <optional>
#include <ostream>

namespace ebike {

template <typename T, std::size_t Capacity>
class StaticQueue {
    static_assert(Capacity > 0, "a queue needs room for at least one item");

    std::array<T, Capacity> items_{};
    std::size_t head_{0};
    std::size_t count_{0};
    std::size_t rejected_{0};

public:
    bool push(const T& item) {
        if (count_ == Capacity) {
            ++rejected_;
            return false;
        }
        items_[(head_ + count_) % Capacity] = item;
        ++count_;
        return true;
    }

    std::optional<T> pop() {
        if (count_ == 0) {
            return std::nullopt;
        }
        T item = items_[head_];
        head_ = (head_ + 1) % Capacity;
        --count_;
        return item;
    }

    std::size_t size() const { return count_; }
    bool empty() const { return count_ == 0; }
    std::size_t rejected() const { return rejected_; }

    friend std::ostream& operator<<(std::ostream& os, const StaticQueue& queue) {
        return os << "StaticQueue{" << queue.count_ << "/" << Capacity << ", rejected=" << queue.rejected_ << "}";
    }
};

}

#endif
