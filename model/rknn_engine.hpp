#pragma once

#include "pipeai/error.hpp"
#include "pipeai/types.hpp"

#include <string>
#include <vector>

namespace pipeai {

class RknnEngine {
public:
    RknnEngine() = default;
    ~RknnEngine();

    Status load(const std::string& model_path);
    Status infer(const ModelInput& input, std::vector<float>& output);
    void close();

    bool ready() const { return initialized_; }
    int input_count() const { return input_count_; }
    int output_count() const { return output_count_; }

private:
    bool initialized_{false};
    std::vector<unsigned char> model_buffer_;
    int input_count_{0};
    int output_count_{0};
#ifdef PIPEAI_WITH_RKNN
    unsigned long ctx_{0};
#endif
};

} // namespace pipeai
