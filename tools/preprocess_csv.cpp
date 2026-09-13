#include "preprocess/csv_reader.hpp"
#include "preprocess/preprocessor.hpp"

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "usage: pipeai_preprocess_csv INPUT.csv [OUTPUT.csv]\n";
        return EXIT_FAILURE;
    }

    const std::string input_path = argv[1];
    const std::string output_path = argc == 3 ? argv[2] : std::string{};

    pipeai::CsvReader reader;
    std::vector<pipeai::RawSample> samples;
    auto status = reader.read(input_path, samples);
    if (!status) {
        std::cerr << "CSV error: " << status.message << '\n';
        return EXIT_FAILURE;
    }

    pipeai::ModelInput tensor{};
    pipeai::PreprocessReport report{};
    pipeai::Preprocessor preprocessor({150.0F});
    status = preprocessor.run(samples, tensor, &report);
    if (!status) {
        std::cerr << "preprocess error: " << status.message << '\n';
        return EXIT_FAILURE;
    }

    std::ostream* out = &std::cout;
    std::ofstream file;
    if (!output_path.empty()) {
        file.open(output_path);
        if (!file) {
            std::cerr << "cannot create output: " << output_path << '\n';
            return EXIT_FAILURE;
        }
        out = &file;
    }

    *out << "D1,D2,D3,D4,D5,D6,D7,D8,IMU\n";
    *out << std::setprecision(9);
    for (std::size_t t = 0; t < pipeai::kTargetLength; ++t) {
        for (std::size_t c = 0; c < pipeai::kInputChannels; ++c) {
            if (c) *out << ',';
            *out << tensor.at(t, c);
        }
        *out << '\n';
    }

    std::cerr << "source_samples=" << report.source_samples
              << " repaired_radar_values=" << report.radar_invalid_values
              << " all_invalid_channels=" << report.all_invalid_radar_channels
              << " imu_nonfinite=" << report.imu_nonfinite_values
              << " savgol=" << (report.savgol_applied ? "yes" : "no")
              << '\n';
    return EXIT_SUCCESS;
}
