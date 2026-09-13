#pragma once

#include "pipeai/types.hpp"
#include <string>

namespace pipeai {

class ISensor {
public:
    virtual ~ISensor() = default;
    virtual bool open(const std::string& path) = 0;
    virtual bool read(RawSample& sample) = 0;
    virtual void close() = 0;
};

}
