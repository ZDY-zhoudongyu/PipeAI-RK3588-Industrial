#pragma once

#include "pipeai/error.hpp"

#include <string>
#include <vector>

namespace pipeai {

class ModelLoader {
public:
    static Status load_file(const std::string& path,
                            std::vector<unsigned char>& data);
};

} // namespace pipeai
