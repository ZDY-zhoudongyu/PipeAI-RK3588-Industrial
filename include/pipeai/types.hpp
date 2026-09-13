#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace pipeai {

inline constexpr std::size_t kRadarChannels = 8;
inline constexpr std::size_t kInputChannels = 9;
inline constexpr std::size_t kTargetLength = 128;
inline constexpr std::size_t kClassCount = 5;

using TimestampNs = std::uint64_t;
using RadarVector = std::array<float, kRadarChannels>;

// One synchronized acquisition sample. The acquisition layer is responsible
// for assigning a monotonic timestamp and preserving D1..D8 physical order.
struct RawSample {
    TimestampNs timestamp_ns{0};
    RadarVector radar{};
    float imu{0.0F};
};

// CPU-side model input. Memory is contiguous in [T, C] row-major order and
// therefore can be passed directly as [1,128,9] float32 to RKNN.
struct ModelInput {
    std::array<float, kTargetLength * kInputChannels> values{};

    float& at(std::size_t t, std::size_t c) {
        return values.at(t * kInputChannels + c);
    }

    const float& at(std::size_t t, std::size_t c) const {
        return values.at(t * kInputChannels + c);
    }

    float* data() noexcept { return values.data(); }
    const float* data() const noexcept { return values.data(); }
    static constexpr std::size_t element_count() noexcept {
        return kTargetLength * kInputChannels;
    }
    static constexpr std::size_t byte_size() noexcept {
        return element_count() * sizeof(float);
    }
};

struct InferenceResult {
    std::uint64_t window_id{0};
    TimestampNs window_start_ns{0};
    TimestampNs window_end_ns{0};
    std::array<float, kClassCount> logits{};
    std::array<float, kClassCount> probabilities{};
    int class_id{-1};
    float confidence{0.0F};
    bool input_healthy{true};
};

static_assert(sizeof(float) == 4, "PipeAI assumes IEEE-754 32-bit float");
static_assert(ModelInput::element_count() == 1152, "Unexpected model input size");

} // namespace pipeai
