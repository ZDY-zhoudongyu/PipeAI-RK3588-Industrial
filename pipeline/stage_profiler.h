#pragma once
#include <chrono>
class StageProfiler { public: void begin(){s=std::chrono::steady_clock::now();} double end(){auto e=std::chrono::steady_clock::now(); return std::chrono::duration<double,std::milli>(e-s).count();} private: std::chrono::steady_clock::time_point s;};
