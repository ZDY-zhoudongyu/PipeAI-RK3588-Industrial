#include "model/model_loader.hpp"

#include <fstream>

namespace pipeai {

Status ModelLoader::load_file(const std::string& path,
                              std::vector<unsigned char>& data) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return Status::failure(ErrorCode::IoError,
                               "cannot open model: " + path);
    }

    const auto size = file.tellg();
    if (size <= 0) {
        return Status::failure(ErrorCode::IoError,
                               "empty model: " + path);
    }

    data.resize(static_cast<std::size_t>(size));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(data.data()), size);

    if (!file) {
        data.clear();
        return Status::failure(ErrorCode::IoError,
                               "failed reading model: " + path);
    }
    return Status::success();
}

} // namespace pipeai
