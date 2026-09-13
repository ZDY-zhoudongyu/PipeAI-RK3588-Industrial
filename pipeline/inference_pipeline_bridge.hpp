#pragma once
#include "pipeai/types.hpp"
#include "preprocess/preprocessor.hpp"
#include "model/rknn_engine.hpp"
#include <string>
#include <vector>

namespace pipeai {

class InferencePipelineBridge
{
public:
    bool initialize(const char* model);
    bool run(const std::vector<RawSample>& samples, InferenceResult& result);
    void release();

private:
    Preprocessor preprocessor_{};
    RknnEngine engine_{};
    bool ready_{false};
};

}
