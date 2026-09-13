#pragma once
#include <cstdint>
namespace pipeai {
struct FrameContext {
    uint64_t id{0};
    double sensor_ms{0};
    double preprocess_ms{0};
    double tensor_ms{0};
    double rknn_ms{0};
    double post_ms{0};
    double total_ms{0};
};
}
