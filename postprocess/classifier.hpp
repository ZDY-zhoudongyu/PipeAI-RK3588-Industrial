#pragma once
#include "pipeai/types.hpp"
#include <string>
namespace pipeai {
class Classifier { public: static void classify(InferenceResult& r); static const char* label(int id); };
}
