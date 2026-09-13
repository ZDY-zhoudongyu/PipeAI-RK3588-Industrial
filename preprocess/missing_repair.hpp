#pragma once

#include <cstddef>
#include <vector>

namespace pipeai {

struct RadarRepairStats {
    std::size_t invalid_values{0};
    bool all_invalid{false};
};

// Python-equivalent radar sanitation:
//   non-finite or <= 0 -> missing
//   all missing        -> sensor_min
//   otherwise          -> np.interp over sample index, including endpoint hold
RadarRepairStats repair_radar_channel(std::vector<float>& values, float sensor_min);

// Python-equivalent IMU sanitation: only non-finite values are replaced by 0.
std::size_t sanitize_imu(std::vector<float>& values);

} // namespace pipeai
