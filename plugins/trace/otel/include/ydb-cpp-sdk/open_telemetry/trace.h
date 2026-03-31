#pragma once

#include <ydb-cpp-sdk/client/trace/trace.h>

#include <opentelemetry/nostd/shared_ptr.h>

namespace opentelemetry::trace {
class TracerProvider;
}

namespace NYdb::inline V3::NTrace {

std::shared_ptr<ITraceProvider> CreateOtelTraceProvider(
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider);

} // namespace NYdb::NTrace
