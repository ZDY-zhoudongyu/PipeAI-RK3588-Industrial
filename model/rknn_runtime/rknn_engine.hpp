#pragma once
#include <vector>
#include <string>

class RknnEngine {
public:
    bool load(const std::string& model_path);
    bool infer(const std::vector<int8_t>& input, std::vector<int8_t>& output);
    void release();
};
