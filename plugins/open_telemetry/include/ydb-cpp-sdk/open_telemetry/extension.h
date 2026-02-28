#pragma once

#include <ydb-cpp-sdk/open_telemetry/metrics.h>
#include <ydb-cpp-sdk/open_telemetry/trace.h>

namespace NYdb::inline V3::NMetrics {

inline void AddOpenTelemetry(TDriverConfig& config
    , opentelemetry::nostd::shared_ptr<opentelemetry::metrics::MeterProvider> meterProvider
    , opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider
) {
    AddOpenTelemetryMetrics(config, std::move(meterProvider));
    AddOpenTelemetryTrace(config, std::move(tracerProvider));
}

} // namespace NYdb::NMetrics
