#include "runtime/application.hpp"
#include "runtime/logger.hpp"
#include "sensor/csv_sensor.hpp"

#include <chrono>
#include <thread>

namespace pipeai {

Status Application::initialize(const std::string& config_path) {
    auto status = config_.load(config_path);
    if (!status) {
        return status;
    }

    Logger::instance().info("Application initialize");
    Logger::instance().info("model=" + config_.get_string("model.path", "models/pipe_model.rknn"));
    Logger::instance().info("window=" + std::to_string(config_.get_size("pipeline.window", 128)));

    // Create pipeline lifecycle here. Sensor feeding and model injection remain
    // separate so deployment can replace input backends without changing app.
    const auto capacity = config_.get_size("pipeline.raw_queue_capacity", 1024);
    pipeline_ = std::make_unique<Pipeline>(capacity);
    pipeline_->set_result_callback([](const InferenceResult& result) {
        Logger::instance().info("result class=" + std::to_string(result.class_id) +
                                " confidence=" + std::to_string(result.confidence));
    });

    if (!pipeline_->start()) {
        return Status::error("pipeline start failed");
    }

    running_ = true;

    const auto csv = config_.get_string("sensor.csv", "");
    if (!csv.empty()) {
        Logger::instance().info("csv replay enabled: " + csv);
        CsvSensor sensor;
        if (sensor.open(csv)) {
            RawSample sample;
            while (sensor.read(sample) && running_) {
                pipeline_->submit(sample);
            }
        }
    }

    return Status::ok();
}

int Application::run() {
    Logger::instance().info("PipeAI runtime loop started");
    while (running_) {
        // Production loop placeholder: workers and sensor callbacks are attached here.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}

void Application::shutdown() {
    running_ = false;
    if (pipeline_) {
        pipeline_->stop();
    }
    Logger::instance().info("PipeAI runtime shutdown");
}

} // namespace pipeai
