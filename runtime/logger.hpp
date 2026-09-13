#pragma once

#include <fstream>
#include <mutex>
#include <string>

namespace pipeai {

enum class LogLevel { Debug = 0, Info, Warn, Error };

class Logger {
public:
    static Logger& instance();

    void set_level(LogLevel level);
    bool set_file(const std::string& path);
    void clear_file();

    void log(LogLevel level, const std::string& message);

    void debug(const std::string& message) { log(LogLevel::Debug, message); }
    void info(const std::string& message) { log(LogLevel::Info, message); }
    void warn(const std::string& message) { log(LogLevel::Warn, message); }
    void error(const std::string& message) { log(LogLevel::Error, message); }

private:
    Logger() = default;
    static const char* level_name(LogLevel level);
    static std::string now_string();

    std::mutex mutex_;
    LogLevel min_level_{LogLevel::Info};
    std::ofstream file_;
};

} // namespace pipeai
