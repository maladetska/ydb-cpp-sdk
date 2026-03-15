#pragma once

#include <ydb-cpp-sdk/client/trace/trace.h>

#include <opentelemetry/nostd/shared_ptr.h>
<<<<<<< HEAD
<<<<<<< HEAD
#include <opentelemetry/trace/tracer_provider.h>
=======
=======
>>>>>>> dcae6d69e (fixes and add metric tests)

namespace opentelemetry::trace {
class TracerProvider;
}
<<<<<<< HEAD
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
>>>>>>> dcae6d69e (fixes and add metric tests)

namespace NYdb::inline V3::NMetrics {

std::shared_ptr<ITraceProvider> CreateOtelTraceProvider(
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider);

} // namespace NYdb::NMetrics
