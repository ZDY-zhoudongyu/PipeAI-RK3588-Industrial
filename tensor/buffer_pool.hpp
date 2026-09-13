#pragma once
#include <cstddef>
#include <mutex>
#include <vector>

namespace pipeai {

class BufferPool {
public:
    BufferPool(std::size_t buffer_size, std::size_t count);
    ~BufferPool();

    float* acquire();
    void release(float* ptr);

private:
    std::size_t size_;
    std::vector<float*> free_;
    std::vector<float*> all_;
    std::mutex mutex_;
};

}
