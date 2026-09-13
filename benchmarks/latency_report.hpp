#pragma once
#include <vector>
#include <algorithm>

struct LatencyReport {
    double average;
    double p95;
    double p99;
};

inline double percentile(std::vector<double> values, double p)
{
    if(values.empty()) return 0;
    std::sort(values.begin(), values.end());
    size_t index=(size_t)(p*values.size());
    if(index>=values.size()) index=values.size()-1;
    return values[index];
}
