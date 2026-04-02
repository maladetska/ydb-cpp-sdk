#pragma once

#include <string>

namespace NYdb::inline V3::NObservability {

inline std::string YdbClientApiAttributeValue(const std::string& clientType) {
    if (clientType == "Query") {
        return "query";
    }
    if (clientType == "Table") {
        return "table";
    }
    if (clientType.empty()) {
        return "unspecified";
    }
    std::string out;
    out.reserve(clientType.size());
    for (unsigned char ch : clientType) {
        out.push_back(static_cast<char>(::tolower(static_cast<int>(ch))));
    }
    return out;
}

} // namespace NYdb::NObservability
