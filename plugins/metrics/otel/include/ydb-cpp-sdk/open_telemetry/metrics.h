#pragma once

#include <ydb-cpp-sdk/client/metrics/metrics.h>

#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/metrics/meter_provider.h>

namespace NYdb::inline V3::NMetrics {

std::shared_ptr<IMetricRegistry> CreateOtelMetricRegistry(
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::MeterProvider> meterProvider);

} // namespace NYdb::NMetrics
