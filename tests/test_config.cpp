#include "runtime/config.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>

int main() {
    const char* path = "pipeai_test_config.ini";
    {
        std::ofstream out(path);
        out << "[preprocess]\n"
               "sensor_min = 150.5\n"
               "enabled = true\n"
               "[pipeline]\n"
               "stride = 16\n";
    }

    pipeai::Config cfg;
    const auto status = cfg.load(path);
    assert(status.ok());
    assert(cfg.get_float("preprocess.sensor_min", 0.0F) == 150.5F);
    assert(cfg.get_bool("preprocess.enabled", false));
    assert(cfg.get_int("pipeline.stride", -1) == 16);
    assert(cfg.get_int("missing.key", 42) == 42);

    std::remove(path);
    std::cout << "config tests passed\n";
    return 0;
}
