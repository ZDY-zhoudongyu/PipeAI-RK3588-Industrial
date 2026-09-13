#include "pipeline/worker.hpp"

namespace pipeai {

Worker::~Worker() { stop(); }

bool Worker::start(Loop loop) {
    if (running_.exchange(true)) return false;
    thread_ = std::thread([this, loop = std::move(loop)]() mutable {
        while (running_) {
            loop();
        }
    });
    return true;
}

void Worker::stop() {
    if (!running_.exchange(false)) return;
    if (thread_.joinable()) thread_.join();
}

} // namespace pipeai
