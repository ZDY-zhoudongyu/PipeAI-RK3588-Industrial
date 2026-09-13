#pragma once

#include <vector>

namespace pipeai {

// Exact algorithmic equivalent of scipy.signal.savgol_filter(x, 7, 2,
// mode="interp") for smoothing (deriv=0), specialized to the model contract.
// For fewer than 7 samples the caller must skip filtering, matching Python.
void savgol_7_2_interp(std::vector<float>& values);

} // namespace pipeai
