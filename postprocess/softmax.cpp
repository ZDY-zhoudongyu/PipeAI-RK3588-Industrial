#include "postprocess/softmax.hpp"
#include <cmath>
#include <algorithm>
namespace pipeai {
std::array<float,kClassCount> Softmax::apply(const std::array<float,kClassCount>& logits){
 float m=*std::max_element(logits.begin(),logits.end()); float s=0; std::array<float,kClassCount> p{};
 for(size_t i=0;i<kClassCount;i++){p[i]=std::exp(logits[i]-m);s+=p[i];}
 if(s>0) for(auto &v:p)v/=s; return p;
}
}
