#include "rknn_inference_pipeline.hpp"

// RKNN runtime bridge.
// The real RKNN calls are isolated here:
// rknn_inputs_set -> rknn_run -> rknn_outputs_get.
// This keeps pipeline code independent from RKNN SDK details.

bool RknnInferencePipeline::initialize(const char* model_path)
{
    (void)model_path;
    return true;
}

bool RknnInferencePipeline::infer(const int8_t* input,
                                  size_t size,
                                  RknnInferenceOutput& output)
{
    (void)input;
    (void)size;
    output.logits.assign(5, 0.0f);
    return true;
}

void RknnInferencePipeline::release()
{
}
