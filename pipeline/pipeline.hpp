#pragma once

#include "pipeline/blocking_queue.hpp"
#include "pipeline/worker.hpp"
#include "pipeai/types.hpp"
#include "preprocess/preprocessor.hpp"
#include "model/rknn_engine.hpp"
#include "postprocess/classifier.hpp"

#include <functional>
#include <memory>
#include <vector>
#include <mutex>

namespace pipeai {

class Pipeline {
public:
    using ResultCallback = std::function<void(const InferenceResult&)>;

    explicit Pipeline(std::size_t queue_capacity = 128);
    ~Pipeline();

    void set_result_callback(ResultCallback cb);
    bool start();
    void stop();

    bool submit(const RawSample& sample);

private:
    void preprocess_loop();
    void inference_loop();
    void post_loop();

    BlockingQueue<RawSample> raw_queue_;
    BlockingQueue<ModelInput> tensor_queue_;
    BlockingQueue<InferenceResult> result_queue_;

    Worker preprocess_worker_;
    Worker inference_worker_;
    Worker post_worker_;

    ResultCallback callback_;

    Preprocessor preprocessor_;
    RknnEngine engine_;

    std::vector<RawSample> window_buffer_;
    std::mutex window_mutex_;
    std::uint64_t window_id_{0};
};

} // namespace pipeai
