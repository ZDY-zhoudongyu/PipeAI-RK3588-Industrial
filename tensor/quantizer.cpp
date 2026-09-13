#include "tensor/quantizer.hpp"
#include <algorithm>
#include <cmath>
namespace pipeai {
int8_t Quantizer::quantize(float v) const {int x=static_cast<int>(std::round(v/scale_))+zero_; return static_cast<int8_t>(std::max(-128,std::min(127,x)));}
float Quantizer::dequantize(int8_t v) const{return (static_cast<int>(v)-zero_)*scale_;}
void Quantizer::quantize(const std::vector<float>& in,std::vector<int8_t>& out) const {out.resize(in.size()); for(size_t i=0;i<in.size();++i) out[i]=quantize(in[i]);}
}
