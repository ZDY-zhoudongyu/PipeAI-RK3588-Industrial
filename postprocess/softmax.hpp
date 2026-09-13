#pragma once
#include <array>
#include "pipeai/types.hpp"
namespace pipeai {
class Softmax { public: static std::array<float,kClassCount> apply(const std::array<float,kClassCount>& logits); };
}
