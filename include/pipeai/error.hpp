#pragma once

#include <string>
#include <utility>

namespace pipeai {

enum class ErrorCode {
    Ok = 0,
    InvalidArgument,
    IoError,
    ParseError,
    QueueClosed,
    ModelLoadFailed,
    RknnInitFailed,
    RknnQueryFailed,
    RknnInferenceFailed,
    PreprocessFailed,
};

struct Status {
    ErrorCode code{ErrorCode::Ok};
    std::string message{};

    bool ok() const noexcept { return code == ErrorCode::Ok; }
    explicit operator bool() const noexcept { return ok(); }

    static Status success() { return {}; }
    static Status failure(ErrorCode c, std::string m) {
        return Status{c, std::move(m)};
    }
};

} // namespace pipeai
