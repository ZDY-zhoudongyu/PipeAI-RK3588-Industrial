#include "sensor_manager.hpp"

namespace pipeai {

bool SensorManager::start()
{
    return sensor_ ? sensor_->open() : false;
}

bool SensorManager::read(RawSample& sample)
{
    return sensor_ && sensor_->read(sample);
}

void SensorManager::stop()
{
    if(sensor_) sensor_->close();
}

} // namespace pipeai
