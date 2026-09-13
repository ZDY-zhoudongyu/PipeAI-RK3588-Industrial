#include "runtime/logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

namespace pipeai {

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::set_level(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    min_level_ = level;
}

bool Logger::set_file(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    file_.close();
    file_.clear();
    file_.open(path, std::ios::out | std::ios::app);
    return file_.is_open();
}

void Logger::clear_file() {
    std::lock_guard<std::mutex> lock(mutex_);
    file_.close();
    file_.clear();
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (static_cast<int>(level) < static_cast<int>(min_level_)) {
        return;
    }

    std::ostringstream line;
    line << now_string() << " [" << level_name(level) << "]"
         << " [tid=" << std::this_thread::get_id() << "] " << message;

    std::ostream& console = (level == LogLevel::Error) ? std::cerr : std::cout;
    console << line.str() << '\n';
    console.flush();

    if (file_.is_open()) {
        file_ << line.str() << '\n';
        file_.flush();
    }
}

const char* Logger::level_name(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}

std::string Logger::now_string() {
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    const std::time_t tt = system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

} // namespace pipeai
