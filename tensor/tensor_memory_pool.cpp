#include "tensor_memory_pool.hpp"
namespace pipeai {
bool TensorMemoryPool::allocate(size_t bytes,size_t count){buffers_.resize(count);for(auto& b:buffers_)b.resize(bytes);return true;}
void* TensorMemoryPool::acquire(){return buffers_.empty()?nullptr:buffers_[0].data();}
void TensorMemoryPool::release(void*){}
}
