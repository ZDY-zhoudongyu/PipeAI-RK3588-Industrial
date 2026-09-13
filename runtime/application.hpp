#pragma once

#include "runtime/config.hpp"
#include "pipeline/pipeline.hpp"
#include <atomic>
#include <string>
#include <memory>

namespace pipeai {

class Application {
public:
    Status initialize(const std::string& config_path);
    int run();
    void shutdown();

private:
    Config config_;
    std::atomic<bool> running_{false};
    std::unique_ptr<Pipeline> pipeline_;
};

} // namespace pipeai
