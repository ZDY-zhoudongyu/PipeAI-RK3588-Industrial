#include <algorithm>
#include <vector>

double percentile(std::vector<double> v, double p)
{
    if(v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    size_t idx = static_cast<size_t>(p*v.size());
    if(idx>=v.size()) idx=v.size()-1;
    return v[idx];
}
