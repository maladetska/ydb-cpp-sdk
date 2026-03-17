#include <ydb-cpp-sdk/open_telemetry/metrics.h>
#include <ydb-cpp-sdk/client/resources/ydb_resources.h>

#include <opentelemetry/common/key_value_iterable_view.h>
#include <opentelemetry/context/runtime_context.h>
#include <opentelemetry/metrics/meter.h>
#include <opentelemetry/metrics/meter_provider.h>
#include <opentelemetry/metrics/sync_instruments.h>
#include <opentelemetry/sdk/metrics/meter_context.h>
#include <opentelemetry/sdk/metrics/meter_provider.h>

#include <unordered_set>

namespace NYdb::inline V3::NMetrics {

namespace {

namespace otel_metrics = opentelemetry::metrics;
namespace otel_nostd = opentelemetry::nostd;
namespace otel_common = opentelemetry::common;
namespace otel_context = opentelemetry::context;
namespace otel_sdk_metrics = opentelemetry::sdk::metrics;

otel_common::KeyValueIterableView<TLabels> MakeAttributes(const TLabels& labels) {
    return otel_common::KeyValueIterableView<TLabels>(labels);
}

class TOtelCounter : public ICounter {
public:
    TOtelCounter(otel_nostd::shared_ptr<otel_metrics::Counter<uint64_t>> counter, const TLabels& labels)
        : Counter_(std::move(counter))
        , Labels_(labels)
    {}

    void Inc() override {
        Counter_->Add(1, MakeAttributes(Labels_), otel_context::RuntimeContext::GetCurrent());
    }

private:
    otel_nostd::shared_ptr<otel_metrics::Counter<uint64_t>> Counter_;
    TLabels Labels_;
};

class TOtelUpDownCounterGauge : public IGauge {
public:
    TOtelUpDownCounterGauge(otel_nostd::shared_ptr<otel_metrics::UpDownCounter<double>> counter, const TLabels& labels)
        : Counter_(std::move(counter))
        , Labels_(labels)
    {}

    void Add(double delta) override {
        Counter_->Add(delta, MakeAttributes(Labels_), otel_context::RuntimeContext::GetCurrent());
        Value_ += delta;
    }

    void Set(double value) override {
        Counter_->Add(value - Value_, MakeAttributes(Labels_), otel_context::RuntimeContext::GetCurrent());
        Value_ = value;
    }

private:
    otel_nostd::shared_ptr<otel_metrics::UpDownCounter<double>> Counter_;
    TLabels Labels_;
    double Value_ = 0;
};

class TOtelHistogram : public IHistogram {
public:
    TOtelHistogram(otel_nostd::shared_ptr<otel_metrics::Histogram<double>> histogram, const TLabels& labels)
        : Histogram_(std::move(histogram))
        , Labels_(labels)
    {}

    void Record(double value) override {
        Histogram_->Record(value, MakeAttributes(Labels_), otel_context::RuntimeContext::GetCurrent());
    }

private:
    otel_nostd::shared_ptr<otel_metrics::Histogram<double>> Histogram_;
    TLabels Labels_;
};

class TOtelMetricRegistry : public IMetricRegistry {
public:
    TOtelMetricRegistry(otel_nostd::shared_ptr<otel_metrics::MeterProvider> meterProvider)
        : MeterProvider_(std::move(meterProvider))
        , Meter_(MeterProvider_->GetMeter("ydb-cpp-sdk", GetSdkSemver()))
    {}

    std::shared_ptr<ICounter> Counter(const std::string& name
        , const TLabels& labels
        , const std::string& description
        , const std::string& unit
    ) override {
        auto counter = Meter_->CreateUInt64Counter(name, description, unit);
        return std::make_shared<TOtelCounter>(std::move(counter), labels);
    }

    std::shared_ptr<IGauge> Gauge(const std::string& name
        , const TLabels& labels
        , const std::string& description
        , const std::string& unit
    ) override {
        auto counter = Meter_->CreateDoubleUpDownCounter(name, description, unit);
        return std::make_shared<TOtelUpDownCounterGauge>(std::move(counter), labels);
    }

    std::shared_ptr<IHistogram> Histogram(const std::string& name
        , const std::vector<double>& buckets
        , const TLabels& labels
        , const std::string& description
        , const std::string& unit
    ) override {
        ConfigureHistogramBuckets(name, unit, buckets);
        auto histogram = Meter_->CreateDoubleHistogram(name, description, unit);
        return std::make_shared<TOtelHistogram>(std::move(histogram), labels);
    }

private:
    void ConfigureHistogramBuckets(const std::string& name, const std::string& unit, const std::vector<double>& buckets) {
        if (buckets.empty()) {
            return;
        }

        auto* sdkProvider = dynamic_cast<otel_sdk_metrics::MeterProvider*>(MeterProvider_.get());
        if (!sdkProvider) {
            return;
        }

        {
            std::lock_guard lock(HistogramViewsLock_);
            if (!HistogramViews_.insert(name).second) {
                return;
            }
        }

        auto selector = std::make_unique<otel_sdk_metrics::InstrumentSelector>(
            otel_sdk_metrics::InstrumentType::kHistogram,
            name,
            unit
        );
        auto meterSelector = std::make_unique<otel_sdk_metrics::MeterSelector>(
            std::string("ydb-cpp-sdk"),
            std::string(GetSdkSemver()),
            std::string()
        );

        auto histogramConfig = std::make_shared<otel_sdk_metrics::HistogramAggregationConfig>();
        histogramConfig->boundaries_ = buckets;

        auto view = std::make_unique<otel_sdk_metrics::View>(
            std::string(),
            std::string(),
            otel_sdk_metrics::AggregationType::kHistogram,
            histogramConfig
        );

        sdkProvider->AddView(std::move(selector), std::move(meterSelector), std::move(view));
    }

    otel_nostd::shared_ptr<otel_metrics::MeterProvider> MeterProvider_;
    otel_nostd::shared_ptr<otel_metrics::Meter> Meter_;
    std::mutex HistogramViewsLock_;
    std::unordered_set<std::string> HistogramViews_;
};

} // namespace

std::shared_ptr<IMetricRegistry> CreateOtelMetricRegistry(
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::MeterProvider> meterProvider)
{
    return std::make_shared<TOtelMetricRegistry>(std::move(meterProvider));
}

} // namespace NYdb::NMetrics
