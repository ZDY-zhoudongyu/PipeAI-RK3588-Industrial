#pragma once
#include <cstdint>
namespace pipeai { int8_t quantize_int8(float x,float scale,int zero_point); float dequantize_int8(int8_t x,float scale,int zero_point); }
