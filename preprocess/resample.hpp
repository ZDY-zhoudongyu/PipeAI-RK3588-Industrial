#pragma once

#include "pipeai/error.hpp"

#include <cstddef>
#include <vector>

namespace pipeai {

// Equivalent to:
// old_x = np.linspace(0, 1, len(src), dtype=np.float32)
// new_x = np.linspace(0, 1, target_len, dtype=np.float32)
// np.interp(new_x, old_x, src).astype(np.float32)
Status resample_linear_normalized(
    const std::vector<float>& source,
    std::size_t target_length,
    std::vector<float>& output);

} // namespace pipeai
