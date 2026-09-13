#pragma once
#include <vector>
namespace pipeai {
struct LatencyStats {
    double average;
    double p95;
    double p99;
};
LatencyStats calculate_latency(std::vector<double> values);
}
