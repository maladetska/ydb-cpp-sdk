#include "query_metrics.h"

#include <iostream>

namespace NYdb::inline V3::NQuery {

namespace {

void SafeLogMetricsError(const char* message) noexcept {
    try {
        try {
            std::cerr << "TQueryMetrics: " << message << ": " << CurrentExceptionMessage() << std::endl;
            return;
        } catch (...) {
        }
        std::cerr << "TQueryMetrics: " << message << ": (unknown)" << std::endl;
    } catch (...) {
    }
}

} // namespace

static const std::vector<double> LatencyBuckets = {
    1, 2, 5, 10, 25, 50, 100, 250, 500, 1000, 2500, 5000, 10000, 30000
};

TQueryMetrics::TQueryMetrics(std::shared_ptr<NMetrics::IMetricRegistry> registry, const std::string& operationName) {
    if (!registry) {
        return;
    }

    try {
        NMetrics::TLabels labels = {{"operation", operationName}};
        RequestCounter_ = registry->Counter("ydb.query.requests", labels);
        ErrorCounter_ = registry->Counter("ydb.query.errors", labels);
        LatencyHistogram_ = registry->Histogram("ydb.query.latency_ms", LatencyBuckets, labels);

        RequestCounter_->Inc();
        StartTime_ = TInstant::Now();
    } catch (...) {
        SafeLogMetricsError("failed to initialize metrics");
        RequestCounter_.reset();
        ErrorCounter_.reset();
        LatencyHistogram_.reset();
    }
}

TQueryMetrics::~TQueryMetrics() noexcept {
    End(EStatus::CLIENT_INTERNAL_ERROR);
}

void TQueryMetrics::End(EStatus status) noexcept {
    if (Ended_) {
        return;
    }
    Ended_ = true;

    try {
        if (LatencyHistogram_) {
            auto durationMs = (TInstant::Now() - StartTime_).MilliSeconds();
            LatencyHistogram_->Record(static_cast<double>(durationMs));
        }
        if (status != EStatus::SUCCESS && ErrorCounter_) {
            ErrorCounter_->Inc();
        }
    } catch (...) {
        SafeLogMetricsError("failed to record metrics");
    }
}

} // namespace NYdb::NQuery
