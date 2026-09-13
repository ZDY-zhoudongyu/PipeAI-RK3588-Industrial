#include "watchdog.hpp"
#include <chrono>
namespace pipeai {
static long long now_ms(){return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();}
void Watchdog::heartbeat(){last_ms_.store(now_ms());}
bool Watchdog::healthy(std::chrono::milliseconds timeout) const{return now_ms()-last_ms_.load() <= timeout.count();}
}
