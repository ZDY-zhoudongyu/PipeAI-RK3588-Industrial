#pragma once

#include "pipeai/error.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>

namespace pipeai {

// Small dependency-free INI reader. Section keys are stored as "section.key".
// It intentionally rejects malformed lines instead of silently ignoring them.
class Config {
public:
    Status load(const std::string& path);

    bool has(const std::string& key) const;
    std::string get_string(const std::string& key, const std::string& fallback = "") const;
    int get_int(const std::string& key, int fallback) const;
    std::size_t get_size(const std::string& key, std::size_t fallback) const;
    float get_float(const std::string& key, float fallback) const;
    bool get_bool(const std::string& key, bool fallback) const;

private:
    static std::string trim(std::string s);
    std::unordered_map<std::string, std::string> values_;
};

} // namespace pipeai
