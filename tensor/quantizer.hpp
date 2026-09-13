#pragma once
#include <cstdint>
#include <vector>
namespace pipeai {
class Quantizer {
public:
 Quantizer(float scale, int32_t zero):scale_(scale),zero_(zero){}
 int8_t quantize(float v) const;
 float dequantize(int8_t v) const;
 void quantize(const std::vector<float>& in,std::vector<int8_t>& out) const;
private: float scale_; int32_t zero_;
};
}
