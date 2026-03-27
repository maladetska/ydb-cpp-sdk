#pragma once

#include <ydb-cpp-sdk/client/metrics/metrics.h>
#include <ydb-cpp-sdk/client/types/status_codes.h>

#include <chrono>
#include <memory>
#include <string>

namespace NYdb::inline V3::NObservability {

class TClientMetrics {
public:
    TClientMetrics(std::shared_ptr<NMetrics::IMetricRegistry> registry,
        const std::string& prefix, const std::string& operationName);
    ~TClientMetrics() noexcept;

    void End(EStatus status) noexcept;

private:
    std::shared_ptr<NMetrics::ICounter> RequestCounter_;
    std::shared_ptr<NMetrics::ICounter> ErrorCounter_;
    std::shared_ptr<NMetrics::IHistogram> DurationHistogram_;
    std::chrono::steady_clock::time_point StartTime_;
    bool Ended_ = false;
};

} // namespace NYdb::NObservability
