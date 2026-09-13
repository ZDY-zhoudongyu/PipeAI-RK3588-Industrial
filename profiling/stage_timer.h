#pragma once
#include <chrono>
namespace pipeai { class StageTimer { public: void start(){t=std::chrono::steady_clock::now();} double stop_ms(){auto e=std::chrono::steady_clock::now(); return std::chrono::duration<double,std::milli>(e-t).count();} private: std::chrono::steady_clock::time_point t;}; }
