#include "preprocess/csv_reader.hpp"
#include "preprocess/preprocessor.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#ifndef PIPEAI_TEST_DATA_DIR
#error "PIPEAI_TEST_DATA_DIR must be defined by CMake"
#endif

int main() {
    const std::string data_dir = PIPEAI_TEST_DATA_DIR;

    pipeai::CsvReader reader;
    std::vector<pipeai::RawSample> input_samples;
    auto status = reader.read(data_dir + "/preprocess_input.csv", input_samples);
    if (!status) {
        std::cerr << status.message << '\n';
        return 1;
    }

    pipeai::ModelInput actual{};
    pipeai::PreprocessReport report{};
    pipeai::Preprocessor preprocessor({150.0F});
    status = preprocessor.run(input_samples, actual, &report);
    if (!status) {
        std::cerr << status.message << '\n';
        return 1;
    }

    std::vector<pipeai::RawSample> expected_rows;
    status = reader.read(data_dir + "/preprocess_expected.csv", expected_rows);
    if (!status) {
        std::cerr << status.message << '\n';
        return 1;
    }
    assert(expected_rows.size() == pipeai::kTargetLength);

    float max_abs_error = 0.0F;
    std::size_t max_t = 0;
    std::size_t max_c = 0;
    for (std::size_t t = 0; t < pipeai::kTargetLength; ++t) {
        for (std::size_t c = 0; c < pipeai::kRadarChannels; ++c) {
            const float err = std::fabs(actual.at(t, c) - expected_rows[t].radar[c]);
            if (err > max_abs_error) {
                max_abs_error = err;
                max_t = t;
                max_c = c;
            }
        }
        const float imu_err = std::fabs(actual.at(t, 8) - expected_rows[t].imu);
        if (imu_err > max_abs_error) {
            max_abs_error = imu_err;
            max_t = t;
            max_c = 8;
        }
    }

    // The oracle uses SciPy's float32 filtering kernels.  Our fixed rational
    // SG weights are mathematically equivalent; this tolerance only covers
    // floating-point accumulation-order differences, not algorithmic drift.
    constexpr float kTolerance = 2.0e-4F;
    if (max_abs_error > kTolerance) {
        std::cerr << "golden mismatch: max_abs_error=" << max_abs_error
                  << " at t=" << max_t << " c=" << max_c
                  << " actual=" << actual.at(max_t, max_c) << '\n';
        return 2;
    }

    assert(report.source_samples == input_samples.size());
    assert(report.savgol_applied);
    assert(report.all_invalid_radar_channels == 1);
    assert(report.radar_invalid_values > 0);
    assert(report.imu_nonfinite_values == 2);

    std::cout << "preprocess golden test passed; max_abs_error="
              << max_abs_error << '\n';
    return 0;
}
