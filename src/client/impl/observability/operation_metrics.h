#pragma once

#include <ydb-cpp-sdk/client/metrics/metrics.h>
#include <ydb-cpp-sdk/client/types/status_codes.h>

#include <library/cpp/logger/log.h>

#include <chrono>
#include <memory>
#include <string>

namespace NYdb::inline V3::NObservability {

class TOperationMetrics {
public:
    TOperationMetrics(std::shared_ptr<NMetrics::IMetricRegistry> registry
        , const std::string& operationName
        , const TLog& log
    );
    ~TOperationMetrics() noexcept;

    void End(EStatus status) noexcept;

private:
    std::shared_ptr<NMetrics::IMetricRegistry> Registry_;
    std::string OperationName_;
    std::shared_ptr<NMetrics::ICounter> RequestCounter_;
    std::shared_ptr<NMetrics::ICounter> ErrorCounter_;
    std::chrono::steady_clock::time_point StartTime_;
    bool Ended_ = false;
    TLog Log_;
};

} // namespace NYdb::NObservability
