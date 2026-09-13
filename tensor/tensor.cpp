#include "tensor/tensor.hpp"
namespace pipeai {
static size_t type_size(TensorType t){return t==TensorType::FLOAT32?4:1;}
Tensor::Tensor(std::vector<int> s, TensorType t):shape_(s),type_(t){resize(s);}
void Tensor::resize(std::vector<int> s){shape_=s; buffer_.resize(elements()*type_size(type_));}
size_t Tensor::elements() const {size_t n=1; for(int v:shape_) n*=v; return shape_.empty()?0:n;}
size_t Tensor::bytes() const{return buffer_.size();}
}
