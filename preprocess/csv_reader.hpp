#pragma once

#include "pipeai/error.hpp"
#include "pipeai/types.hpp"

#include <string>
#include <vector>

namespace pipeai {

// Offline CSV adapter used for validation and commissioning.  The production
// acquisition path will create RawSample objects directly, so preprocessing is
// shared between CSV tests and live sensors.
class CsvReader {
public:
    Status read(const std::string& path, std::vector<RawSample>& samples) const;

private:
    static Status split_line(const std::string& line, std::vector<std::string>& fields);
    static std::string trim(std::string s);
    static Status parse_float(const std::string& text, float& value);
};

} // namespace pipeai
