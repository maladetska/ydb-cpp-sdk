#include "client_metrics.h"

#include <exception>

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

std::string StatusToString(EStatus status) {
    switch (status) {
        case EStatus::SUCCESS:                  return "SUCCESS";
        case EStatus::BAD_REQUEST:              return "BAD_REQUEST";
        case EStatus::UNAUTHORIZED:             return "UNAUTHORIZED";
        case EStatus::INTERNAL_ERROR:           return "INTERNAL_ERROR";
        case EStatus::ABORTED:                  return "ABORTED";
        case EStatus::UNAVAILABLE:              return "UNAVAILABLE";
        case EStatus::OVERLOADED:               return "OVERLOADED";
        case EStatus::SCHEME_ERROR:             return "SCHEME_ERROR";
        case EStatus::GENERIC_ERROR:            return "GENERIC_ERROR";
        case EStatus::TIMEOUT:                  return "TIMEOUT";
        case EStatus::BAD_SESSION:              return "BAD_SESSION";
        case EStatus::PRECONDITION_FAILED:      return "PRECONDITION_FAILED";
        case EStatus::ALREADY_EXISTS:           return "ALREADY_EXISTS";
        case EStatus::NOT_FOUND:                return "NOT_FOUND";
        case EStatus::SESSION_EXPIRED:          return "SESSION_EXPIRED";
        case EStatus::CANCELLED:                return "CANCELLED";
        case EStatus::UNDETERMINED:             return "UNDETERMINED";
        case EStatus::UNSUPPORTED:              return "UNSUPPORTED";
        case EStatus::SESSION_BUSY:             return "SESSION_BUSY";
        case EStatus::EXTERNAL_ERROR:           return "EXTERNAL_ERROR";
        case EStatus::TRANSPORT_UNAVAILABLE:    return "TRANSPORT_UNAVAILABLE";
        case EStatus::CLIENT_RESOURCE_EXHAUSTED:return "CLIENT_RESOURCE_EXHAUSTED";
        case EStatus::CLIENT_DEADLINE_EXCEEDED: return "CLIENT_DEADLINE_EXCEEDED";
        case EStatus::CLIENT_INTERNAL_ERROR:    return "CLIENT_INTERNAL_ERROR";
        case EStatus::CLIENT_CANCELLED:         return "CLIENT_CANCELLED";
        case EStatus::CLIENT_UNAUTHENTICATED:   return "CLIENT_UNAUTHENTICATED";
        case EStatus::CLIENT_CALL_UNIMPLEMENTED:return "CLIENT_CALL_UNIMPLEMENTED";
        case EStatus::CLIENT_OUT_OF_RANGE:      return "CLIENT_OUT_OF_RANGE";
        case EStatus::CLIENT_DISCOVERY_FAILED:  return "CLIENT_DISCOVERY_FAILED";
        case EStatus::CLIENT_LIMITS_REACHED:    return "CLIENT_LIMITS_REACHED";
        default:                                return "STATUS_UNDEFINED";
    }
}

} // namespace

static const std::vector<double> DurationBucketsSec = {
    0.001, 0.005, 0.01, 0.05, 0.1, 0.5, 1, 5, 10
};

TClientMetrics::TClientMetrics(std::shared_ptr<NMetrics::IMetricRegistry> registry,
    const std::string& prefix, const std::string& operationName)
{
    if (!registry) {
        return;
    }

    try {
        NMetrics::TLabels labels = {{"operation", operationName}};
        RequestCounter_ = registry->Counter(prefix + ".requests", labels);
        ErrorCounter_ = registry->Counter(prefix + ".errors", labels);

        NMetrics::TLabels durationLabels = {
            {"db.system.name", "ydb"},
            {"db.operation.name", operationName},
        };
        DurationHistogram_ = registry->Histogram("db.client.operation.duration", DurationBucketsSec, durationLabels);

        RequestCounter_->Inc();
        StartTime_ = std::chrono::steady_clock::now();
    } catch (...) {
        SafeLogMetricsError("failed to initialize metrics");
        RequestCounter_.reset();
        ErrorCounter_.reset();
        DurationHistogram_.reset();
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
        if (DurationHistogram_) {
            auto elapsed = std::chrono::steady_clock::now() - StartTime_;
            double durationSec = std::chrono::duration<double>(elapsed).count();
            DurationHistogram_->Record(durationSec);
        }

        if (status != EStatus::SUCCESS && ErrorCounter_) {
            ErrorCounter_->Inc();
        }
    } catch (...) {
        SafeLogMetricsError("failed to record metrics");
    }
}

} // namespace NYdb::NObservability
