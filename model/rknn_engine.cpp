#include "model/rknn_engine.hpp"
#include "model/model_loader.hpp"

#include <cstring>

#ifdef PIPEAI_WITH_RKNN
#include <rknn_api.h>
#endif

namespace pipeai {

RknnEngine::~RknnEngine() {
    close();
}

Status RknnEngine::load(const std::string& model_path) {
    Status s = ModelLoader::load_file(model_path, model_buffer_);
    if (!s) {
        return s;
    }

#ifndef PIPEAI_WITH_RKNN
    return Status::failure(ErrorCode::RknnInitFailed,
                           "RKNN runtime disabled. Build with -DPIPEAI_WITH_RKNN=ON");
#else
    rknn_context context = 0;
    int ret = rknn_init(&context,
                        model_buffer_.data(),
                        model_buffer_.size(),
                        0,
                        nullptr);
    if (ret != RKNN_SUCC) {
        return Status::failure(ErrorCode::RknnInitFailed,
                               "rknn_init failed");
    }

    ctx_ = context;

    rknn_input_output_num io_num{};
    ret = rknn_query(context,
                     RKNN_QUERY_IN_OUT_NUM,
                     &io_num,
                     sizeof(io_num));
    if (ret != RKNN_SUCC) {
        close();
        return Status::failure(ErrorCode::RknnQueryFailed,
                               "rknn_query input/output failed");
    }

    input_count_ = io_num.n_input;
    output_count_ = io_num.n_output;
    initialized_ = true;

    return Status::success();
#endif
}

Status RknnEngine::infer(const ModelInput& input,
                         std::vector<float>& output) {
    if (!initialized_) {
        return Status::failure(ErrorCode::RknnInferenceFailed,
                               "RKNN engine not initialized");
    }

#ifndef PIPEAI_WITH_RKNN
    (void)input;
    (void)output;
    return Status::failure(ErrorCode::RknnInferenceFailed,
                           "RKNN runtime disabled");
#else
    rknn_context context = static_cast<rknn_context>(ctx_);

    rknn_input in{};
    in.index = 0;
    in.type = RKNN_TENSOR_FLOAT32;
    in.fmt = RKNN_TENSOR_NCHW;
    in.size = ModelInput::byte_size();
    in.buf = const_cast<float*>(input.data());

    int ret = rknn_inputs_set(context, 1, &in);
    if (ret != RKNN_SUCC) {
        return Status::failure(ErrorCode::RknnInferenceFailed,
                               "rknn_inputs_set failed");
    }

    ret = rknn_run(context, nullptr);
    if (ret != RKNN_SUCC) {
        return Status::failure(ErrorCode::RknnInferenceFailed,
                               "rknn_run failed");
    }

    std::vector<rknn_output> outputs(output_count_);
    std::memset(outputs.data(), 0, sizeof(rknn_output) * outputs.size());

    for (auto& item : outputs) {
        item.want_float = 1;
    }

    ret = rknn_outputs_get(context,
                           output_count_,
                           outputs.data(),
                           nullptr);
    if (ret != RKNN_SUCC) {
        return Status::failure(ErrorCode::RknnInferenceFailed,
                               "rknn_outputs_get failed");
    }

    output.clear();
    if (output_count_ > 0 && outputs[0].buf != nullptr) {
        auto* ptr = static_cast<float*>(outputs[0].buf);
        const auto count = outputs[0].size / sizeof(float);
        output.assign(ptr, ptr + count);
    }

    rknn_outputs_release(context, output_count_, outputs.data());

    return Status::success();
#endif
}

void RknnEngine::close() {
#ifdef PIPEAI_WITH_RKNN
    if (ctx_ != 0) {
        rknn_destroy(static_cast<rknn_context>(ctx_));
        ctx_ = 0;
    }
#endif
    initialized_ = false;
    input_count_ = 0;
    output_count_ = 0;
    model_buffer_.clear();
}

} // namespace pipeai
