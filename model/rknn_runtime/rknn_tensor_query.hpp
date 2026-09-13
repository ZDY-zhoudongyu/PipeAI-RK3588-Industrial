#pragma once
namespace pipeai {
struct RknnTensorInfo {
 int index{0};
 int dims[4]{1,128,9,0};
 float scale{1.0f};
 int zero_point{0};
};
}
