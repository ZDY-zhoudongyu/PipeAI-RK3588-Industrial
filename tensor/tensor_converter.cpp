#include "tensor_converter.hpp"
#include <cmath>
#include <algorithm>
namespace pipeai {
void TensorConverter::floatToInt8(const float* s,int8_t* d,size_t n,float scale,int zp){
 for(size_t i=0;i<n;i++){ int q=(int)std::round(s[i]/scale)+zp; q=std::max(-128,std::min(127,q)); d[i]=(int8_t)q; }
}
void TensorConverter::int8ToFloat(const int8_t* s,float* d,size_t n,float scale,int zp){
 for(size_t i=0;i<n;i++) d[i]=((int)s[i]-zp)*scale;
}
}
