#include "inference_pipeline_bridge.hpp"
#include "postprocess/classifier.hpp"
#include <chrono>

namespace pipeai {

bool InferencePipelineBridge::initialize(const char* model)
{
    if (model == nullptr) return false;
    ready_ = static_cast<bool>(engine_.load(model));
    return ready_;
}

bool InferencePipelineBridge::run(const std::vector<RawSample>& samples, InferenceResult& result)
{
    if (!ready_) return false;

    ModelInput input{};
    if (!preprocessor_.run(samples, input)) return false;

    std::vector<float> logits;
    if (!engine_.infer(input, logits)) return false;
    if (logits.size() < kClassCount) return false;

    for (size_t i = 0; i < kClassCount; ++i) {
        result.logits[i] = logits[i];
    }
    Classifier::classify(result);
    if (!samples.empty()) {
        result.window_start_ns = samples.front().timestamp_ns;
        result.window_end_ns = samples.back().timestamp_ns;
    }
    return true;
}

void InferencePipelineBridge::release()
{
    engine_.close();
    ready_ = false;
}

}
