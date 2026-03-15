#pragma once

<<<<<<< HEAD
#include <src/client/impl/observability/client_metrics.h>

namespace NYdb::inline V3::NQuery {

class TQueryMetrics : public NObservability::TClientMetrics {
public:
    TQueryMetrics(std::shared_ptr<NMetrics::IMetricRegistry> registry, const std::string& operationName)
        : TClientMetrics(std::move(registry), operationName)
    {}
=======
#include <ydb-cpp-sdk/client/metrics/metrics.h>
#include <ydb-cpp-sdk/client/types/status_codes.h>

#include <util/datetime/base.h>

#include <memory>
#include <string>

namespace NYdb::inline V3::NQuery {

class TQueryMetrics {
public:
    TQueryMetrics(std::shared_ptr<NMetrics::IMetricRegistry> registry, const std::string& operationName);
    ~TQueryMetrics() noexcept;

    void End(EStatus status) noexcept;

private:
    std::shared_ptr<NMetrics::ICounter> RequestCounter_;
    std::shared_ptr<NMetrics::ICounter> ErrorCounter_;
    std::shared_ptr<NMetrics::IHistogram> LatencyHistogram_;
    TInstant StartTime_;
    bool Ended_ = false;
>>>>>>> 1ca4253b5 (fixes and add metric tests)
};

} // namespace NYdb::NQuery
