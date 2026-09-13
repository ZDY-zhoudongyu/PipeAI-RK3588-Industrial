#include "preprocess/missing_repair.hpp"

#include <cmath>
#include <limits>

namespace pipeai {

RadarRepairStats repair_radar_channel(std::vector<float>& values, float sensor_min) {
    RadarRepairStats stats{};
    if (values.empty()) return stats;

    for (float& value : values) {
        if (!std::isfinite(value) || value <= 0.0F) {
            value = std::numeric_limits<float>::quiet_NaN();
            ++stats.invalid_values;
        }
    }

    std::size_t first_valid = values.size();
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!std::isnan(values[i])) {
            first_valid = i;
            break;
        }
    }

    if (first_valid == values.size()) {
        for (float& value : values) value = sensor_min;
        stats.all_invalid = true;
        return stats;
    }

    // np.interp uses the first valid value for missing points to the left.
    for (std::size_t i = 0; i < first_valid; ++i) {
        values[i] = values[first_valid];
    }

    std::size_t left = first_valid;
    while (left + 1 < values.size()) {
        std::size_t right = left + 1;
        while (right < values.size() && std::isnan(values[right])) ++right;

        if (right == values.size()) {
            // np.interp uses the last valid value for missing points to the right.
            for (std::size_t i = left + 1; i < values.size(); ++i) {
                values[i] = values[left];
            }
            break;
        }

        if (right > left + 1) {
            const double y0 = static_cast<double>(values[left]);
            const double y1 = static_cast<double>(values[right]);
            const double span = static_cast<double>(right - left);
            for (std::size_t i = left + 1; i < right; ++i) {
                const double alpha = static_cast<double>(i - left) / span;
                values[i] = static_cast<float>(y0 + (y1 - y0) * alpha);
            }
        }
        left = right;
    }

    return stats;
}

std::size_t sanitize_imu(std::vector<float>& values) {
    std::size_t count = 0;
    for (float& value : values) {
        if (!std::isfinite(value)) {
            value = 0.0F;
            ++count;
        }
    }
    return count;
}

} // namespace pipeai
