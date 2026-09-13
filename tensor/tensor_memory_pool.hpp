#pragma once
#include <vector>
#include <cstddef>
namespace pipeai {
class TensorMemoryPool {
public:
 bool allocate(size_t bytes, size_t count);
 void* acquire();
 void release(void* p);
private:
 std::vector<std::vector<unsigned char>> buffers_;
};
}
