#pragma once
#include <string>
struct RknnTensorInfo { int index{0}; int dims[4]{0}; int n_dims{0}; float scale{1.0f}; int zero_point{0}; std::string name; };
