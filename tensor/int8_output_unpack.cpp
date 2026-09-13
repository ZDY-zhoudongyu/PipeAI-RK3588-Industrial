#include "int8_output_unpack.hpp"

namespace pipeai {
void dequant_int8(const int8_t* input,float* output,int size,float scale,int zero_point)
{
    for(int i=0;i<size;i++)
        output[i]=(static_cast<int>(input[i])-zero_point)*scale;
}
}
