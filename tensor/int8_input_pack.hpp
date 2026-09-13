#pragma once
#include <cstdint>
void pack_float_to_int8(const float* src,int8_t* dst,int size,float scale,int zp);
