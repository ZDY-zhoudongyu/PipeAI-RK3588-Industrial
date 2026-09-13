#pragma once
#include <string>
#include <vector>

namespace pipeai {

struct RknnRuntimeTensorInfo {
    int index{0};
    std::vector<int> dims;
    int type{0};
    float scale{1.0f};
    int zero_point{0};
};

class RknnRuntimeInfo {
public:
    bool query(void* context);
    const std::vector<RknnRuntimeTensorInfo>& inputs() const { return inputs_; }
    const std::vector<RknnRuntimeTensorInfo>& outputs() const { return outputs_; }
private:
    std::vector<RknnRuntimeTensorInfo> inputs_;
    std::vector<RknnRuntimeTensorInfo> outputs_;
};

}
