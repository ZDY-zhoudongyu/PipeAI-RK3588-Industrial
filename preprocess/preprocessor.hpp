#pragma once

#include "pipeai/error.hpp"
#include "pipeai/types.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace pipeai {

struct PreprocessOptions {
    float sensor_min{150.0F};
};

struct PreprocessReport {
    std::size_t source_samples{0};
    std::size_t radar_invalid_values{0};
    std::size_t imu_nonfinite_values{0};
    std::size_t all_invalid_radar_channels{0};
    bool savgol_applied{false};
};

class Preprocessor {
public:
    explicit Preprocessor(PreprocessOptions options = {});

    // Produces the exact external tensor contract consumed by the exported
    // End2EndPipeNet: contiguous float32 [128,9].  Model-internal baseline,
    // normalization, differences, spatial projection and dual-branch features
    // MUST NOT be repeated here.
    Status run(
        const std::vector<RawSample>& samples,
        ModelInput& output,
        PreprocessReport* report = nullptr) const;

private:
    PreprocessOptions options_;
};

} // namespace pipeai
