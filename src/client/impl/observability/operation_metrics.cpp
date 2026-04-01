#include "operation_metrics.h"

#include <src/client/topic/common/log_lazy.h>

#include <exception>

namespace NYdb::inline V3::NObservability {

namespace {

void SafeLogMetricsError(TLog& log, const char* message) noexcept {
    try {
        try {
            std::rethrow_exception(std::current_exception());
        } catch (const std::exception& e) {
            LOG_LAZY(log, TLOG_ERR, std::string("TOperationMetrics: ") + message + ": " + e.what());
            return;
        } catch (...) {
        }
        LOG_LAZY(log, TLOG_ERR, std::string("TOperationMetrics: ") + message + ": (unknown)");
    } catch (...) {
    }
}

} // namespace

TOperationMetrics::TOperationMetrics(NSdkStats::TStatCollector::TClientOperationStatCollector* operationCollector
    , const std::string& operationName
    , const TLog& log
) : Collector_(operationCollector)
    , OperationName_(operationName)
    , Log_(log)
{
    if (!Collector_) {
        return;
    }
    try {
        Collector_->IncRequestCount(operationName);
        StartTime_ = std::chrono::steady_clock::now();
    } catch (...) {
        SafeLogMetricsError(Log_, "failed to initialize metrics");
        Collector_ = nullptr;
    }
}

TOperationMetrics::~TOperationMetrics() noexcept {
    End(EStatus::CLIENT_INTERNAL_ERROR);
}

void TOperationMetrics::End(EStatus status) noexcept {
    if (Ended_) {
        return;
    }
    Ended_ = true;

    if (!Collector_) {
        return;
    }

    try {
        auto elapsed = std::chrono::steady_clock::now() - StartTime_;
        double durationSec = std::chrono::duration<double>(elapsed).count();
        Collector_->RecordLatency(OperationName_, durationSec, status);
        Collector_->IncErrorCount(OperationName_, status);
    } catch (...) {
        SafeLogMetricsError(Log_, "failed to record metrics");
    }
}

} // namespace NYdb::NObservability
