#pragma once
#include <vector>
#include <cstdint>

struct RknnInferenceOutput {
    std::vector<float> logits;
};

class RknnInferencePipeline {
public:
    bool initialize(const char* model_path);
    bool infer(const int8_t* input, size_t size, RknnInferenceOutput& output);
    void release();
};
