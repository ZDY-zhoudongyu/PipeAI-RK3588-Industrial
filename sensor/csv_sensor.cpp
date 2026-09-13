#include "sensor/csv_sensor.hpp"
#include <sstream>

namespace pipeai {

bool CsvSensor::open(const std::string& path) {
    file_.open(path);
    return file_.good();
}

bool CsvSensor::read(RawSample& sample) {
    if (!file_.good()) return false;
    std::string line;
    if (!std::getline(file_, line)) return false;
    std::stringstream ss(line);
    char comma;
    ss >> sample.timestamp_ns;
    for (size_t i=0;i<kRadarChannels;i++) {
        ss >> comma >> sample.radar[i];
    }
    ss >> comma >> sample.imu;
    return !ss.fail();
}

void CsvSensor::close() {
    if (file_.is_open()) file_.close();
}

}
