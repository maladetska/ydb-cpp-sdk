#pragma once

#include <ydb-cpp-sdk/client/trace/trace.h>

#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/trace/tracer_provider.h>

namespace NYdb::inline V3::NMetrics {

std::shared_ptr<ITraceProvider> CreateOtelTraceProvider(
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider);

} // namespace NYdb::NMetrics
