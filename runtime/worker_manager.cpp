#include "runtime/worker_manager.hpp"

namespace pipeai {

bool WorkerManager::add(const std::string& name, Worker::Loop loop) {
    auto w = std::make_unique<Worker>();
    if (!w->start(loop)) {
        return false;
    }
    workers_.push_back({name, std::move(w)});
    return true;
}

bool WorkerManager::start_all() {
    return true;
}

void WorkerManager::stop_all() {
    for (auto& item : workers_) {
        if (item.worker) item.worker->stop();
    }
}

bool WorkerManager::healthy() const {
    for (const auto& item : workers_) {
        if (item.worker && !item.worker->running()) return false;
    }
    return true;
}

} // namespace pipeai
