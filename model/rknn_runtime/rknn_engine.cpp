#include "rknn_engine.hpp"
#include <iostream>

bool RknnEngine::load(const std::string& model_path){
    std::cout << "Load RKNN model: " << model_path << std::endl;
    return true;
}

bool RknnEngine::infer(const std::vector<int8_t>& input, std::vector<int8_t>& output){
    // RKNN runtime path:
    // rknn_inputs_set -> rknn_run -> rknn_outputs_get
    output.resize(5);
    return true;
}

void RknnEngine::release(){}
