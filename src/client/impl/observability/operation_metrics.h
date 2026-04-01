#pragma once

#include <src/client/impl/stats/stats.h>
#include <ydb-cpp-sdk/client/types/status_codes.h>

#include <library/cpp/logger/log.h>

#include <chrono>
#include <string>

namespace NYdb::inline V3::NObservability {

class TOperationMetrics {
public:
    TOperationMetrics(NSdkStats::TStatCollector::TClientOperationStatCollector* operationCollector
        , const std::string& operationName
        , const TLog& log
    );
    ~TOperationMetrics() noexcept;

    void End(EStatus status) noexcept;

private:
    NSdkStats::TStatCollector::TClientOperationStatCollector* Collector_ = nullptr;
    std::string OperationName_;
    std::chrono::steady_clock::time_point StartTime_{};
    bool Ended_ = false;
    TLog Log_;
};

} // namespace NYdb::NObservability
