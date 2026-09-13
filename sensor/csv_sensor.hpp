#pragma once

#include "pipeai/types.hpp"
#include "sensor/sensor_interface.hpp"
#include <string>
#include <fstream>

namespace pipeai {

class CsvSensor : public ISensor {
public:
    bool open(const std::string& path) override;
    bool read(RawSample& sample) override;
void close() override;

private:
    std::ifstream file_;
};

}
