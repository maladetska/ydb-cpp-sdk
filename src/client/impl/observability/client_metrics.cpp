#include "client_metrics.h"

#include <exception>
#include <util/string/cast.h>

namespace NYdb::inline V3::NObservability {

namespace {

void SafeLogMetricsError(const char* /*message*/) noexcept {
    try {
        try {
            std::rethrow_exception(std::current_exception());
        } catch (const std::exception&) {
            return;
        } catch (...) {
        }
    } catch (...) {
    }
}

} // namespace

static const std::vector<double> DurationBucketsSec = {
    0.001, 0.005, 0.01, 0.05, 0.1, 0.5, 1, 5, 10
};

static constexpr const char* RequestsDescription = "Number of database client operations started.";
static constexpr const char* ErrorsDescription = "Number of database client operations that failed.";
static constexpr const char* DurationDescription = "Duration of database client operations.";

TClientMetrics::TClientMetrics(std::shared_ptr<NMetrics::IMetricRegistry> registry
    , const std::string& operationName
) : Registry_(std::move(registry))
    , OperationName_(operationName)
{
    if (!Registry_) {
        return;
    }

    try {
        NMetrics::TLabels labels = {
            {"db.system.name", "other_sql"},
            {"db.operation.name", operationName},
        };
        RequestCounter_ = Registry_->Counter("db.client.operation.requests", labels, RequestsDescription, "{operation}");
        ErrorCounter_ = Registry_->Counter("db.client.operation.errors", labels, ErrorsDescription, "{error}");

        RequestCounter_->Inc();
        StartTime_ = std::chrono::steady_clock::now();
    } catch (...) {
        SafeLogMetricsError("failed to initialize metrics");
        RequestCounter_.reset();
        ErrorCounter_.reset();
        Registry_.reset();
    }
}

TClientMetrics::~TClientMetrics() noexcept {
    End(EStatus::CLIENT_INTERNAL_ERROR);
}

void TClientMetrics::End(EStatus status) noexcept {
    if (Ended_) {
        return;
    }
    Ended_ = true;

    try {
        const std::string statusCode = ToString(status);
        if (Registry_) {
            auto elapsed = std::chrono::steady_clock::now() - StartTime_;
            double durationSec = std::chrono::duration<double>(elapsed).count();
            NMetrics::TLabels durationLabels = {
                {"db.system.name", "other_sql"},
                {"db.operation.name", OperationName_},
                {"db.response.status_code", statusCode},
            };
            if (status != EStatus::SUCCESS) {
                durationLabels["error.type"] = statusCode;
            }
            auto durationHistogram = Registry_->Histogram(
                "db.client.operation.duration",
                DurationBucketsSec,
                durationLabels,
                DurationDescription,
                "s");
            if (durationHistogram) {
                durationHistogram->Record(durationSec);
            }
        }

        if (status != EStatus::SUCCESS && ErrorCounter_) {
            ErrorCounter_->Inc();
        }
    } catch (...) {
        SafeLogMetricsError("failed to record metrics");
    }
}

} // namespace NYdb::NObservability
