#pragma once
#include <string>
namespace pipeai {
class RknnContext {
public:
 bool init(const std::string& path);
 void release();
 bool valid() const;
private:
 void* ctx_{nullptr};
};
}
