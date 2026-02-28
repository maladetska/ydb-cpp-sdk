#pragma once

#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/metrics/metrics.h>

#include <opentelemetry/trace/tracer_provider.h>

namespace NYdb::inline V3::NMetrics {

class TOtelTraceProvider : public ITraceProvider {
public:
    TOtelTraceProvider(opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider);

    std::shared_ptr<ITracer> GetTracer(const std::string& name) override;

private:
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> TracerProvider_;
};

inline void AddOpenTelemetryTrace(
    TDriverConfig& config,
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider)
{
    if (tracerProvider) {
        config.SetTraceExporter(std::make_shared<TOtelTraceProvider>(std::move(tracerProvider)));
    }
}

} // namespace NYdb::NMetrics
