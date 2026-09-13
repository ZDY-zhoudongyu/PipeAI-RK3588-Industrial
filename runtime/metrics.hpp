#pragma once
#include <atomic>
namespace pipeai {
class Metrics {
public:
 void inc_frames(){frames_++;}
 unsigned long frames() const{return frames_.load();}
private:
 std::atomic<unsigned long> frames_{0};
};
}
