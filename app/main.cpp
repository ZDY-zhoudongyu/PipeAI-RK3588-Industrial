#include "runtime/config.hpp"
#include "runtime/logger.hpp"
#include "runtime/application.hpp"
#include "pipeai/types.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {
pipeai::LogLevel parse_level(const std::string& s) {
    if (s == "debug") return pipeai::LogLevel::Debug;
    if (s == "warn") return pipeai::LogLevel::Warn;
    if (s == "error") return pipeai::LogLevel::Error;
    return pipeai::LogLevel::Info;
}
}

int main(int argc, char** argv) {
    const std::string config_path = argc > 1 ? argv[1] : "config/pipeai.ini";

    pipeai::Config config;
    const pipeai::Status status = config.load(config_path);
    if (!status) {
        std::cerr << "config error: " << status.message << '\n';
        return EXIT_FAILURE;
    }

    auto& log = pipeai::Logger::instance();
    log.set_level(parse_level(config.get_string("logging.level", "info")));
    const std::string log_file = config.get_string("logging.file", "");
    if (!log_file.empty() && !log.set_file(log_file)) {
        std::cerr << "warning: cannot open log file: " << log_file << '\n';
    }

    log.info("PipeAI-RK3588 bootstrap OK");
    log.info("model input contract: float32 [1,128,9], bytes=" +
             std::to_string(pipeai::ModelInput::byte_size()));
    log.info("window stride=" + std::to_string(config.get_size("pipeline.window_stride", 16)) +
             ", raw_queue_capacity=" + std::to_string(config.get_size("pipeline.raw_queue_capacity", 1024)));
    pipeai::Application app;
    const auto app_status = app.initialize(config_path);
    if (!app_status) {
        log.error("application init failed: " + app_status.message);
        return EXIT_FAILURE;
    }

    const int result = app.run();
    app.shutdown();
    return result;
}
