#pragma once

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

namespace pipeai {

// Bounded MPMC queue with explicit shutdown semantics.
// - push() blocks while full.
// - pop() blocks while empty.
// - close() wakes every waiter. No new values can be pushed after close().
// - values already queued remain drainable after close().
template <typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(std::size_t capacity) : capacity_(capacity) {
        if (capacity_ == 0) {
            throw std::invalid_argument("BlockingQueue capacity must be > 0");
        }
    }

    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue& operator=(const BlockingQueue&) = delete;

    bool push(const T& value) { return emplace_impl(value); }
    bool push(T&& value) { return emplace_impl(std::move(value)); }

    template <class Rep, class Period>
    bool push_for(T value, const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!not_full_.wait_for(lock, timeout, [&] { return closed_ || queue_.size() < capacity_; })) {
            return false;
        }
        if (closed_) {
            return false;
        }
        queue_.emplace_back(std::move(value));
        lock.unlock();
        not_empty_.notify_one();
        return true;
    }

    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [&] { return closed_ || !queue_.empty(); });
        if (queue_.empty()) {
            return std::nullopt;
        }
        T value = std::move(queue_.front());
        queue_.pop_front();
        lock.unlock();
        not_full_.notify_one();
        return value;
    }

    template <class Rep, class Period>
    std::optional<T> pop_for(const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!not_empty_.wait_for(lock, timeout, [&] { return closed_ || !queue_.empty(); })) {
            return std::nullopt;
        }
        if (queue_.empty()) {
            return std::nullopt;
        }
        T value = std::move(queue_.front());
        queue_.pop_front();
        lock.unlock();
        not_full_.notify_one();
        return value;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        not_empty_.notify_all();
        not_full_.notify_all();
    }

    bool closed() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return closed_;
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    std::size_t capacity() const noexcept { return capacity_; }

private:
    template <typename U>
    bool emplace_impl(U&& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [&] { return closed_ || queue_.size() < capacity_; });
        if (closed_) {
            return false;
        }
        queue_.emplace_back(std::forward<U>(value));
        lock.unlock();
        not_empty_.notify_one();
        return true;
    }

    const std::size_t capacity_;
    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    std::deque<T> queue_;
    bool closed_{false};
};

} // namespace pipeai
