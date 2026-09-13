#pragma once
#include "sensor_interface.hpp"
#include <memory>

namespace pipeai {

class SensorManager {
public:
    bool start();
    bool read(RawSample& sample);
    void stop();

private:
    std::unique_ptr<ISensor> sensor_;
};

} // namespace pipeai
