#pragma once
#include "../common/inference_result.hpp"
#include <functional>

using ResultCallback = std::function<void(const InferenceResult&)>;
