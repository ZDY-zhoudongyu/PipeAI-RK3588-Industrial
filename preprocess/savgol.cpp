#include "preprocess/savgol.hpp"

#include <array>
#include <cstddef>

namespace pipeai {
namespace {

// Evaluation weights of the degree-2 least-squares polynomial fitted to seven
// samples at x=0..6.  Rows 0,1,2 reproduce scipy's left mode="interp" edge;
// rows 4,5,6 reproduce the right edge.  Row 3 is the standard SG convolution.
constexpr std::array<std::array<double, 7>, 7> kWeights{{
    {{ 32.0/42.0, 15.0/42.0,  3.0/42.0, -4.0/42.0, -6.0/42.0, -3.0/42.0,  5.0/42.0 }},
    {{ 15.0/42.0, 12.0/42.0,  9.0/42.0,  6.0/42.0,  3.0/42.0,  0.0,       -3.0/42.0 }},
    {{  3.0/42.0,  9.0/42.0, 12.0/42.0, 12.0/42.0,  9.0/42.0,  3.0/42.0, -6.0/42.0 }},
    {{ -2.0/21.0,  3.0/21.0,  6.0/21.0,  7.0/21.0,  6.0/21.0,  3.0/21.0, -2.0/21.0 }},
    {{ -6.0/42.0,  3.0/42.0,  9.0/42.0, 12.0/42.0, 12.0/42.0,  9.0/42.0,  3.0/42.0 }},
    {{ -3.0/42.0,  0.0,        3.0/42.0,  6.0/42.0,  9.0/42.0, 12.0/42.0, 15.0/42.0 }},
    {{  5.0/42.0, -3.0/42.0, -6.0/42.0, -4.0/42.0,  3.0/42.0, 15.0/42.0, 32.0/42.0 }}
}};

float evaluate(const std::vector<float>& x, std::size_t start, std::size_t weight_row) {
    double acc = 0.0;
    for (std::size_t j = 0; j < 7; ++j) {
        acc += kWeights[weight_row][j] * static_cast<double>(x[start + j]);
    }
    return static_cast<float>(acc);
}

} // namespace

void savgol_7_2_interp(std::vector<float>& values) {
    if (values.size() < 7) return;

    const std::vector<float> source = values;
    const std::size_t n = values.size();

    // scipy mode="interp": fit the first 7 points and evaluate positions 0..2.
    values[0] = evaluate(source, 0, 0);
    values[1] = evaluate(source, 0, 1);
    values[2] = evaluate(source, 0, 2);

    // Standard centered SG smoothing for all full interior windows.
    for (std::size_t i = 3; i + 3 < n; ++i) {
        values[i] = evaluate(source, i - 3, 3);
    }

    // Fit the final 7 points and evaluate their local positions 4..6.
    const std::size_t start = n - 7;
    values[n - 3] = evaluate(source, start, 4);
    values[n - 2] = evaluate(source, start, 5);
    values[n - 1] = evaluate(source, start, 6);
}

} // namespace pipeai
