#pragma once
#include <chrono>

class LatencyMeter
{
public:
    void start()
    {
        begin_ = std::chrono::steady_clock::now();
    }

    double stop_ms()
    {
        auto end = std::chrono::steady_clock::now();
        return std::chrono::duration<double, std::milli>(end-begin_).count();
    }

private:
    std::chrono::steady_clock::time_point begin_;
};
