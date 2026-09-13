#pragma once
#include <cstdint>
#include <vector>
namespace pipeai {
enum class TensorType { FLOAT32, INT8, UINT8 };
class Tensor {
public:
 Tensor()=default;
 Tensor(std::vector<int> shape, TensorType type=TensorType::FLOAT32);
 void resize(std::vector<int> shape);
 size_t elements() const;
 size_t bytes() const;
 std::vector<int> shape() const {return shape_;}
 TensorType type() const {return type_;}
 float* data() {return reinterpret_cast<float*>(buffer_.data());}
 const uint8_t* raw() const {return buffer_.data();}
 std::vector<uint8_t>& raw(){return buffer_;}
private:
 std::vector<int> shape_; TensorType type_{TensorType::FLOAT32}; std::vector<uint8_t> buffer_;
};
}
