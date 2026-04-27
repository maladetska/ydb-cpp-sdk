#pragma once

#include <ydb-cpp-sdk/client/trace/trace.h>

#include <opentelemetry/version.h>
#include <opentelemetry/nostd/shared_ptr.h>

OPENTELEMETRY_BEGIN_NAMESPACE
namespace trace {
class TracerProvider;
}
OPENTELEMETRY_END_NAMESPACE

namespace NYdb::inline V3::NTrace {

std::shared_ptr<ITraceProvider> CreateOtelTraceProvider(
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider);

} // namespace NYdb::NTrace
