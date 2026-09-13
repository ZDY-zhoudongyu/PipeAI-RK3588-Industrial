#pragma once
#include <cstddef>
#include <cstdint>
namespace pipeai {
class TensorConverter {
public:
 static void floatToInt8(const float* src, int8_t* dst, size_t n, float scale, int zero_point);
 static void int8ToFloat(const int8_t* src, float* dst, size_t n, float scale, int zero_point);
};
}
