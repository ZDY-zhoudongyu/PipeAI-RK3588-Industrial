#include "pipeline/pipeline.hpp"
#include <chrono>
#include <mutex>

namespace pipeai {

Pipeline::Pipeline(std::size_t queue_capacity)
    : raw_queue_(queue_capacity),
      tensor_queue_(queue_capacity),
      result_queue_(queue_capacity) {}

Pipeline::~Pipeline() { stop(); }

void Pipeline::set_result_callback(ResultCallback cb) {
    callback_ = std::move(cb);
}

bool Pipeline::start() {
    bool ok = true;
    ok &= preprocess_worker_.start([this]{ preprocess_loop(); });
    ok &= inference_worker_.start([this]{ inference_loop(); });
    ok &= post_worker_.start([this]{ post_loop(); });
    return ok;
}

void Pipeline::stop() {
    raw_queue_.close();
    tensor_queue_.close();
    result_queue_.close();
    preprocess_worker_.stop();
    inference_worker_.stop();
    post_worker_.stop();
}

bool Pipeline::submit(const RawSample& sample) {
    return raw_queue_.push(sample);
}

void Pipeline::preprocess_loop() {
    while (true) {
        auto item = raw_queue_.pop();
        if (!item) break;
        std::vector<RawSample> local_window;
        {
            std::lock_guard<std::mutex> lock(window_mutex_);
            window_buffer_.push_back(*item);
            if (window_buffer_.size() >= kTargetLength) {
                local_window.swap(window_buffer_);
            }
        }
        if (!local_window.empty()) {
            ModelInput input;
            auto status = preprocessor_.run(local_window, input, nullptr);
            if (status) tensor_queue_.push(std::move(input));
        }
    }
}

void Pipeline::inference_loop() {
    while (true) {
        auto item = tensor_queue_.pop();
        if (!item) break;
        std::vector<float> output;
        InferenceResult result;
        result.window_id = ++window_id_;
        if (engine_.ready() && engine_.infer(*item, output)) {
            for (size_t i = 0; i < kClassCount && i < output.size(); ++i)
                result.logits[i] = output[i];
        }
        result_queue_.push(std::move(result));
    }
}

void Pipeline::post_loop() {
    while (true) {
        auto result = result_queue_.pop();
        if (!result) break;
        Classifier::classify(*result);
        if (callback_) callback_(*result);
    }
}

} // namespace pipeai
