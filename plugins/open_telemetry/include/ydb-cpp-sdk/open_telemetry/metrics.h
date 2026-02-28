#pragma once

#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/metrics/metrics.h>

#include <opentelemetry/metrics/meter_provider.h>

namespace NYdb::inline V3::NMetrics {

class TOtelMetricRegistry : public IMetricRegistry {
public:
    TOtelMetricRegistry(opentelemetry::nostd::shared_ptr<opentelemetry::metrics::MeterProvider> meterProvider);

    std::shared_ptr<ICounter> Counter(const std::string& name, const TLabels& labels = {}) override;
    std::shared_ptr<IGauge> Gauge(const std::string& name, const TLabels& labels = {}) override;
    std::shared_ptr<IHistogram> Histogram(const std::string& name, const std::vector<double>& buckets, const TLabels& labels = {}) override;

private:
    void ConfigureHistogramBuckets(const std::string& name, const std::vector<double>& buckets);

    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::MeterProvider> MeterProvider_;
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter> Meter_;
    std::mutex HistogramViewsLock_;
    std::unordered_set<std::string> HistogramViews_;
};

inline void AddOpenTelemetryMetrics(
    TDriverConfig& config,
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::MeterProvider> meterProvider)
{
    if (meterProvider) {
        config.SetMetricExporter(std::make_shared<TOtelMetricRegistry>(std::move(meterProvider)));
    }
}

} // namespace NYdb::NMetrics
