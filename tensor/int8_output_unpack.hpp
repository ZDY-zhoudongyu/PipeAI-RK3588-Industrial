#pragma once
#include <cstdint>

namespace pipeai {
void dequant_int8(
    const int8_t* input,
    float* output,
    int size,
    float scale,
    int zero_point);
}
