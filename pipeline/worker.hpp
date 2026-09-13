#pragma once

#include <atomic>
#include <functional>
#include <thread>

namespace pipeai {

class Worker {
public:
    using Loop = std::function<void()>;
    Worker() = default;
    ~Worker();

    Worker(const Worker&) = delete;
    Worker& operator=(const Worker&) = delete;

    bool start(Loop loop);
    void stop();
    bool running() const noexcept { return running_; }

private:
    std::atomic<bool> running_{false};
    std::thread thread_;
};

} // namespace pipeai
