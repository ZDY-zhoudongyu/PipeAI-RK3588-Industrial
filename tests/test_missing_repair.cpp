#include "preprocess/missing_repair.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

namespace {
bool close(float a, float b, float eps = 1e-6F) {
    return std::fabs(a - b) <= eps;
}
}

int main() {
    using pipeai::repair_radar_channel;

    {
        const float nan = std::numeric_limits<float>::quiet_NaN();
        std::vector<float> x{nan, nan, 10.0F, nan, nan, 16.0F, nan};
        const auto stats = repair_radar_channel(x, 150.0F);
        assert(stats.invalid_values == 5);
        assert(!stats.all_invalid);
        assert(close(x[0], 10.0F));
        assert(close(x[1], 10.0F));
        assert(close(x[2], 10.0F));
        assert(close(x[3], 12.0F));
        assert(close(x[4], 14.0F));
        assert(close(x[5], 16.0F));
        assert(close(x[6], 16.0F));
    }

    {
        std::vector<float> x{0.0F, -1.0F, std::numeric_limits<float>::infinity()};
        const auto stats = repair_radar_channel(x, 150.0F);
        assert(stats.invalid_values == 3);
        assert(stats.all_invalid);
        for (float v : x) assert(close(v, 150.0F));
    }

    {
        std::vector<float> imu{1.0F, std::numeric_limits<float>::quiet_NaN(),
                               -2.0F, std::numeric_limits<float>::infinity()};
        const auto count = pipeai::sanitize_imu(imu);
        assert(count == 2);
        assert(close(imu[0], 1.0F));
        assert(close(imu[1], 0.0F));
        assert(close(imu[2], -2.0F)); // negative IMU is valid in the Python reference
        assert(close(imu[3], 0.0F));
    }

    std::cout << "missing repair tests passed\n";
    return 0;
}
