#pragma once
#include <atomic>
#include <chrono>

namespace pipeai {
class Watchdog {
public:
    void heartbeat();
    bool healthy(std::chrono::milliseconds timeout) const;
private:
    std::atomic<long long> last_ms_{0};
};
}
