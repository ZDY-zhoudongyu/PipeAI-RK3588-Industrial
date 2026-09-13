#pragma once
#include <cstdint>

struct InferenceResult
{
    int class_id = -1;
    float confidence = 0.0f;
    float probability[5] = {0};
    uint64_t timestamp = 0;
};
