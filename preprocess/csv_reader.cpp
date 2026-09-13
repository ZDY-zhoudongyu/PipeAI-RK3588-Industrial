#include "preprocess/csv_reader.hpp"

#include <array>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <sstream>
#include <unordered_map>

namespace pipeai {
namespace {

bool has_utf8_bom(const std::string& s) {
    return s.size() >= 3 &&
           static_cast<unsigned char>(s[0]) == 0xEF &&
           static_cast<unsigned char>(s[1]) == 0xBB &&
           static_cast<unsigned char>(s[2]) == 0xBF;
}

} // namespace

Status CsvReader::read(const std::string& path, std::vector<RawSample>& samples) const {
    std::ifstream in(path);
    if (!in) {
        return Status::failure(ErrorCode::IoError, "cannot open CSV: " + path);
    }

    std::string header_line;
    if (!std::getline(in, header_line)) {
        return Status::failure(ErrorCode::ParseError, "CSV is empty: " + path);
    }
    if (!header_line.empty() && header_line.back() == '\r') {
        header_line.pop_back();
    }
    if (has_utf8_bom(header_line)) {
        header_line.erase(0, 3);
    }

    std::vector<std::string> headers;
    Status status = split_line(header_line, headers);
    if (!status) return status;

    std::unordered_map<std::string, std::size_t> column;
    for (std::size_t i = 0; i < headers.size(); ++i) {
        column[trim(headers[i])] = i;
    }

    std::array<std::size_t, kRadarChannels> radar_columns{};
    for (std::size_t ch = 0; ch < kRadarChannels; ++ch) {
        const std::string name = "D" + std::to_string(ch + 1);
        const auto it = column.find(name);
        if (it == column.end()) {
            return Status::failure(ErrorCode::ParseError,
                "required CSV column missing: " + name);
        }
        radar_columns[ch] = it->second;
    }

    const auto imu_it = column.find("IMU");
    if (imu_it == column.end()) {
        return Status::failure(ErrorCode::ParseError, "required CSV column missing: IMU");
    }
    const std::size_t imu_column = imu_it->second;

    samples.clear();
    std::string line;
    std::size_t line_no = 1;
    while (std::getline(in, line)) {
        ++line_no;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (trim(line).empty()) continue;

        std::vector<std::string> fields;
        status = split_line(line, fields);
        if (!status) {
            status.message += " at CSV line " + std::to_string(line_no);
            return status;
        }

        RawSample sample{};
        sample.timestamp_ns = static_cast<TimestampNs>(samples.size());

        for (std::size_t ch = 0; ch < kRadarChannels; ++ch) {
            const std::size_t idx = radar_columns[ch];
            const std::string text = idx < fields.size() ? fields[idx] : std::string{};
            float value = std::numeric_limits<float>::quiet_NaN();
            status = parse_float(text, value);
            if (!status) {
                status.message += " for D" + std::to_string(ch + 1) +
                                  " at CSV line " + std::to_string(line_no);
                return status;
            }
            sample.radar[ch] = value;
        }

        {
            const std::string text = imu_column < fields.size() ? fields[imu_column] : std::string{};
            float value = std::numeric_limits<float>::quiet_NaN();
            status = parse_float(text, value);
            if (!status) {
                status.message += " for IMU at CSV line " + std::to_string(line_no);
                return status;
            }
            sample.imu = value;
        }

        samples.push_back(sample);
    }

    if (samples.empty()) {
        return Status::failure(ErrorCode::ParseError, "CSV contains no data rows: " + path);
    }
    return Status::success();
}

Status CsvReader::split_line(const std::string& line, std::vector<std::string>& fields) {
    fields.clear();
    std::string field;
    bool quoted = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (quoted) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    field.push_back('"');
                    ++i;
                } else {
                    quoted = false;
                }
            } else {
                field.push_back(c);
            }
        } else {
            if (c == ',') {
                fields.push_back(field);
                field.clear();
            } else if (c == '"') {
                if (!trim(field).empty()) {
                    return Status::failure(ErrorCode::ParseError,
                        "unexpected quote in unquoted CSV field");
                }
                field.clear();
                quoted = true;
            } else {
                field.push_back(c);
            }
        }
    }

    if (quoted) {
        return Status::failure(ErrorCode::ParseError, "unterminated quoted CSV field");
    }
    fields.push_back(field);
    return Status::success();
}

std::string CsvReader::trim(std::string s) {
    const auto first = s.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return {};
    const auto last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, last - first + 1);
}

Status CsvReader::parse_float(const std::string& text, float& value) {
    const std::string s = trim(text);
    if (s.empty()) {
        value = std::numeric_limits<float>::quiet_NaN();
        return Status::success();
    }

    errno = 0;
    char* end = nullptr;
    const float parsed = std::strtof(s.c_str(), &end);
    if (end == s.c_str() || *end != '\0') {
        return Status::failure(ErrorCode::ParseError, "invalid numeric value: '" + s + "'");
    }
    // Overflow to +/-inf is intentionally preserved; the reference Python
    // sanitizer treats non-finite values as invalid data rather than a CSV parse error.
    (void)errno;
    value = parsed;
    return Status::success();
}

} // namespace pipeai
