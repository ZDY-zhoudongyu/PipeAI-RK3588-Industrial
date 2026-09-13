#include "preprocess/resample.hpp"

#include <algorithm>
#include <vector>

namespace pipeai {
namespace {

std::vector<float> linspace01_float32(std::size_t count) {
    std::vector<float> x(count, 0.0F);
    if (count <= 1) return x;
    const double denom = static_cast<double>(count - 1);
    for (std::size_t i = 0; i < count; ++i) {
        // numpy.linspace computes positions in floating point and the requested
        // dtype is float32.  Casting here preserves the float32 knot locations.
        x[i] = static_cast<float>(static_cast<double>(i) / denom);
    }
    x.front() = 0.0F;
    x.back() = 1.0F;
    return x;
}

float interp_one(float x, const std::vector<float>& xp, const std::vector<float>& fp) {
    if (x <= xp.front()) return fp.front();
    if (x >= xp.back()) return fp.back();

    const auto upper = std::upper_bound(xp.begin(), xp.end(), x);
    const std::size_t right = static_cast<std::size_t>(upper - xp.begin());
    const std::size_t left = right - 1;

    const double x0 = static_cast<double>(xp[left]);
    const double x1 = static_cast<double>(xp[right]);
    const double y0 = static_cast<double>(fp[left]);
    const double y1 = static_cast<double>(fp[right]);
    const double xd = static_cast<double>(x);
    const double t = (xd - x0) / (x1 - x0);
    return static_cast<float>(y0 + t * (y1 - y0));
}

} // namespace

Status resample_linear_normalized(
    const std::vector<float>& source,
    std::size_t target_length,
    std::vector<float>& output) {

    if (source.empty()) {
        return Status::failure(ErrorCode::InvalidArgument, "cannot resample an empty sequence");
    }
    if (target_length == 0) {
        return Status::failure(ErrorCode::InvalidArgument, "target_length must be > 0");
    }

    output.resize(target_length);
    if (source.size() == 1) {
        std::fill(output.begin(), output.end(), source.front());
        return Status::success();
    }
    if (target_length == 1) {
        output[0] = source.front();
        return Status::success();
    }

    const std::vector<float> old_x = linspace01_float32(source.size());
    const std::vector<float> new_x = linspace01_float32(target_length);
    for (std::size_t i = 0; i < target_length; ++i) {
        output[i] = interp_one(new_x[i], old_x, source);
    }
    return Status::success();
}

} // namespace pipeai
