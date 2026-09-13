#include "runtime/config.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace pipeai {

Status Config::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        return Status::failure(ErrorCode::IoError, "cannot open config: " + path);
    }

    values_.clear();
    std::string section;
    std::string line;
    std::size_t line_no = 0;

    while (std::getline(in, line)) {
        ++line_no;
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        if (line.front() == '[' && line.back() == ']') {
            section = trim(line.substr(1, line.size() - 2));
            if (section.empty()) {
                return Status::failure(ErrorCode::ParseError,
                    "empty section at line " + std::to_string(line_no));
            }
            continue;
        }

        const auto eq = line.find('=');
        if (eq == std::string::npos) {
            return Status::failure(ErrorCode::ParseError,
                "expected key=value at line " + std::to_string(line_no));
        }

        std::string key = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));
        if (key.empty()) {
            return Status::failure(ErrorCode::ParseError,
                "empty key at line " + std::to_string(line_no));
        }
        if (!section.empty()) {
            key = section + "." + key;
        }
        values_[key] = value;
    }

    return Status::success();
}

bool Config::has(const std::string& key) const {
    return values_.find(key) != values_.end();
}

std::string Config::get_string(const std::string& key, const std::string& fallback) const {
    const auto it = values_.find(key);
    return it == values_.end() ? fallback : it->second;
}

int Config::get_int(const std::string& key, int fallback) const {
    const auto it = values_.find(key);
    if (it == values_.end()) return fallback;
    try {
        std::size_t pos = 0;
        const int value = std::stoi(it->second, &pos, 10);
        if (pos != it->second.size()) return fallback;
        return value;
    } catch (...) {
        return fallback;
    }
}

std::size_t Config::get_size(const std::string& key, std::size_t fallback) const {
    const auto it = values_.find(key);
    if (it == values_.end()) return fallback;
    try {
        std::size_t pos = 0;
        const unsigned long long value = std::stoull(it->second, &pos, 10);
        if (pos != it->second.size() || value > std::numeric_limits<std::size_t>::max()) return fallback;
        return static_cast<std::size_t>(value);
    } catch (...) {
        return fallback;
    }
}

float Config::get_float(const std::string& key, float fallback) const {
    const auto it = values_.find(key);
    if (it == values_.end()) return fallback;
    try {
        std::size_t pos = 0;
        const float value = std::stof(it->second, &pos);
        if (pos != it->second.size()) return fallback;
        return value;
    } catch (...) {
        return fallback;
    }
}

bool Config::get_bool(const std::string& key, bool fallback) const {
    const auto it = values_.find(key);
    if (it == values_.end()) return fallback;
    std::string value = it->second;
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (value == "1" || value == "true" || value == "yes" || value == "on") return true;
    if (value == "0" || value == "false" || value == "no" || value == "off") return false;
    return fallback;
}

std::string Config::trim(std::string s) {
    const auto not_space = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    return s;
}

} // namespace pipeai
