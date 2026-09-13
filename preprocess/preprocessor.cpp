#include "preprocess/preprocessor.hpp"

#include "preprocess/missing_repair.hpp"
#include "preprocess/resample.hpp"
#include "preprocess/savgol.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace pipeai {

Preprocessor::Preprocessor(PreprocessOptions options) : options_(options) {}

Status Preprocessor::run(
    const std::vector<RawSample>& samples,
    ModelInput& output,
    PreprocessReport* report) const {

    if (samples.empty()) {
        return Status::failure(ErrorCode::PreprocessFailed, "preprocess input is empty");
    }
    if (!std::isfinite(options_.sensor_min)) {
        return Status::failure(ErrorCode::InvalidArgument, "sensor_min must be finite");
    }

    PreprocessReport local{};
    local.source_samples = samples.size();

    std::array<std::vector<float>, kRadarChannels> radar;
    std::vector<float> imu;
    imu.reserve(samples.size());
    for (auto& channel : radar) channel.reserve(samples.size());

    for (const RawSample& sample : samples) {
        for (std::size_t ch = 0; ch < kRadarChannels; ++ch) {
            radar[ch].push_back(sample.radar[ch]);
        }
        imu.push_back(sample.imu);
    }

    for (std::size_t ch = 0; ch < kRadarChannels; ++ch) {
        const RadarRepairStats stats = repair_radar_channel(radar[ch], options_.sensor_min);
        local.radar_invalid_values += stats.invalid_values;
        if (stats.all_invalid) ++local.all_invalid_radar_channels;
    }
    local.imu_nonfinite_values = sanitize_imu(imu);

    // Python applies SG smoothing only to radar and only when sequence length >= 7.
    if (samples.size() >= 7) {
        for (auto& channel : radar) savgol_7_2_interp(channel);
        local.savgol_applied = true;
    }

    // Clamp after smoothing exactly like np.maximum(radar, sensor_min).
    for (auto& channel : radar) {
        for (float& value : channel) {
            value = std::max(value, options_.sensor_min);
        }
    }

    std::array<std::vector<float>, kRadarChannels> radar_out;
    for (std::size_t ch = 0; ch < kRadarChannels; ++ch) {
        Status status = resample_linear_normalized(radar[ch], kTargetLength, radar_out[ch]);
        if (!status) return status;
    }
    std::vector<float> imu_out;
    Status status = resample_linear_normalized(imu, kTargetLength, imu_out);
    if (!status) return status;

    for (std::size_t t = 0; t < kTargetLength; ++t) {
        for (std::size_t ch = 0; ch < kRadarChannels; ++ch) {
            output.at(t, ch) = radar_out[ch][t];
        }
        output.at(t, kRadarChannels) = imu_out[t];
    }

    if (report) *report = local;
    return Status::success();
}

} // namespace pipeai
