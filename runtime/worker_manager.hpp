#pragma once

#include <memory>
#include <string>
#include <vector>
#include "pipeline/worker.hpp"

namespace pipeai {

class WorkerManager {
public:
    bool add(const std::string& name, Worker::Loop loop);
    bool start_all();
    void stop_all();
    bool healthy() const;

private:
    struct Entry {
        std::string name;
        std::unique_ptr<Worker> worker;
    };

    std::vector<Entry> workers_;
};

} // namespace pipeai
