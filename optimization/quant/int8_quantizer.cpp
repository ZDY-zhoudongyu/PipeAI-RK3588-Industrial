#include "int8_quantizer.h"
#include <cmath>
namespace pipeai {
int8_t quantize_int8(float x,float scale,int zp){ int q=(int)std::lround(x/scale)+zp; if(q>127)q=127; if(q<-128)q=-128; return (int8_t)q; }
float dequantize_int8(int8_t x,float scale,int zp){ return ((int)x-zp)*scale; }
}
