#include "buffer_pool.hpp"
#include <algorithm>

namespace pipeai {

BufferPool::BufferPool(std::size_t buffer_size, std::size_t count)
    : size_(buffer_size) {
    for (std::size_t i=0;i<count;i++) {
        float* p = new float[size_];
        all_.push_back(p);
        free_.push_back(p);
    }
}

BufferPool::~BufferPool(){
    for(auto p: all_) delete[] p;
}

float* BufferPool::acquire(){
    std::lock_guard<std::mutex> lock(mutex_);
    if(free_.empty()) return nullptr;
    float* p=free_.back();
    free_.pop_back();
    return p;
}

void BufferPool::release(float* ptr){
    if(!ptr) return;
    std::lock_guard<std::mutex> lock(mutex_);
    if(std::find(all_.begin(), all_.end(), ptr)!=all_.end())
        free_.push_back(ptr);
}

}
