#include "int8_input_pack.hpp"
#include <cmath>
void pack_float_to_int8(const float* src,int8_t* dst,int size,float scale,int zp){
    for(int i=0;i<size;i++){
        int v=(int)std::round(src[i]/scale)+zp;
        if(v>127)v=127;
        if(v<-128)v=-128;
        dst[i]=(int8_t)v;
    }
}
