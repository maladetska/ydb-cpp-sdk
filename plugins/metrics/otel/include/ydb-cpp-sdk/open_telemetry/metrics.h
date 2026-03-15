#pragma once

#include <ydb-cpp-sdk/client/metrics/metrics.h>

#include <opentelemetry/nostd/shared_ptr.h>
<<<<<<< HEAD
<<<<<<< HEAD
#include <opentelemetry/metrics/meter_provider.h>
=======
=======
>>>>>>> dcae6d69e (fixes and add metric tests)

namespace opentelemetry::metrics {
class MeterProvider;
}
<<<<<<< HEAD
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
>>>>>>> dcae6d69e (fixes and add metric tests)

namespace NYdb::inline V3::NMetrics {

std::shared_ptr<IMetricRegistry> CreateOtelMetricRegistry(
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::MeterProvider> meterProvider);

} // namespace NYdb::NMetrics
