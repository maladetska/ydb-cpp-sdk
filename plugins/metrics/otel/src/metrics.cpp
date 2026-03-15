#include <ydb-cpp-sdk/open_telemetry/metrics.h>
#include <ydb-cpp-sdk/client/resources/ydb_resources.h>

#include <opentelemetry/common/key_value_iterable_view.h>
#include <opentelemetry/context/runtime_context.h>
#include <opentelemetry/metrics/meter.h>
#include <opentelemetry/metrics/meter_provider.h>
#include <opentelemetry/metrics/sync_instruments.h>
#include <opentelemetry/sdk/metrics/meter_context.h>
#include <opentelemetry/sdk/metrics/meter_provider.h>

<<<<<<< HEAD
#include <unordered_set>

=======
>>>>>>> 1ca4253b5 (fixes and add metric tests)
namespace NYdb::inline V3::NMetrics {

namespace {

<<<<<<< HEAD
namespace otel_metrics = opentelemetry::metrics;
namespace otel_nostd = opentelemetry::nostd;
namespace otel_common = opentelemetry::common;
namespace otel_context = opentelemetry::context;
namespace otel_sdk_metrics = opentelemetry::sdk::metrics;

otel_common::KeyValueIterableView<TLabels> MakeAttributes(const TLabels& labels) {
    return otel_common::KeyValueIterableView<TLabels>(labels);
=======
using namespace opentelemetry;

common::KeyValueIterableView<TLabels> MakeAttributes(const TLabels& labels) {
    return common::KeyValueIterableView<TLabels>(labels);
>>>>>>> 1ca4253b5 (fixes and add metric tests)
}

class TOtelCounter : public ICounter {
public:
<<<<<<< HEAD
    TOtelCounter(otel_nostd::shared_ptr<otel_metrics::Counter<uint64_t>> counter, const TLabels& labels)
=======
    TOtelCounter(nostd::shared_ptr<metrics::Counter<uint64_t>> counter, const TLabels& labels)
>>>>>>> 1ca4253b5 (fixes and add metric tests)
        : Counter_(std::move(counter))
        , Labels_(labels)
    {}

    void Inc() override {
<<<<<<< HEAD
        Counter_->Add(1, MakeAttributes(Labels_), otel_context::RuntimeContext::GetCurrent());
    }

private:
    otel_nostd::shared_ptr<otel_metrics::Counter<uint64_t>> Counter_;
=======
        Counter_->Add(1, MakeAttributes(Labels_), context::RuntimeContext::GetCurrent());
    }

private:
    nostd::shared_ptr<metrics::Counter<uint64_t>> Counter_;
>>>>>>> 1ca4253b5 (fixes and add metric tests)
    TLabels Labels_;
};

class TOtelUpDownCounterGauge : public IGauge {
public:
<<<<<<< HEAD
    TOtelUpDownCounterGauge(otel_nostd::shared_ptr<otel_metrics::UpDownCounter<double>> counter, const TLabels& labels)
=======
    TOtelUpDownCounterGauge(nostd::shared_ptr<metrics::UpDownCounter<double>> counter, const TLabels& labels)
>>>>>>> 1ca4253b5 (fixes and add metric tests)
        : Counter_(std::move(counter))
        , Labels_(labels)
    {}

    void Add(double delta) override {
<<<<<<< HEAD
        Counter_->Add(delta, MakeAttributes(Labels_), otel_context::RuntimeContext::GetCurrent());
=======
        Counter_->Add(delta, MakeAttributes(Labels_), context::RuntimeContext::GetCurrent());
>>>>>>> 1ca4253b5 (fixes and add metric tests)
        Value_ += delta;
    }

    void Set(double value) override {
<<<<<<< HEAD
        Counter_->Add(value - Value_, MakeAttributes(Labels_), otel_context::RuntimeContext::GetCurrent());
=======
        Counter_->Add(value - Value_, MakeAttributes(Labels_), context::RuntimeContext::GetCurrent());
>>>>>>> 1ca4253b5 (fixes and add metric tests)
        Value_ = value;
    }

private:
<<<<<<< HEAD
    otel_nostd::shared_ptr<otel_metrics::UpDownCounter<double>> Counter_;
=======
    nostd::shared_ptr<metrics::UpDownCounter<double>> Counter_;
>>>>>>> 1ca4253b5 (fixes and add metric tests)
    TLabels Labels_;
    double Value_ = 0;
};

class TOtelHistogram : public IHistogram {
public:
<<<<<<< HEAD
    TOtelHistogram(otel_nostd::shared_ptr<otel_metrics::Histogram<double>> histogram, const TLabels& labels)
=======
    TOtelHistogram(nostd::shared_ptr<metrics::Histogram<double>> histogram, const TLabels& labels)
>>>>>>> 1ca4253b5 (fixes and add metric tests)
        : Histogram_(std::move(histogram))
        , Labels_(labels)
    {}

    void Record(double value) override {
<<<<<<< HEAD
        Histogram_->Record(value, MakeAttributes(Labels_), otel_context::RuntimeContext::GetCurrent());
    }

private:
    otel_nostd::shared_ptr<otel_metrics::Histogram<double>> Histogram_;
=======
        Histogram_->Record(value, MakeAttributes(Labels_), context::RuntimeContext::GetCurrent());
    }

private:
    nostd::shared_ptr<metrics::Histogram<double>> Histogram_;
>>>>>>> 1ca4253b5 (fixes and add metric tests)
    TLabels Labels_;
};

class TOtelMetricRegistry : public IMetricRegistry {
public:
<<<<<<< HEAD
    TOtelMetricRegistry(otel_nostd::shared_ptr<otel_metrics::MeterProvider> meterProvider)
=======
    TOtelMetricRegistry(nostd::shared_ptr<metrics::MeterProvider> meterProvider)
>>>>>>> 1ca4253b5 (fixes and add metric tests)
        : MeterProvider_(std::move(meterProvider))
        , Meter_(MeterProvider_->GetMeter("ydb-cpp-sdk", GetSdkSemver()))
    {}

<<<<<<< HEAD
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
=======
    std::shared_ptr<ICounter> Counter(const std::string& name, const TLabels& labels) override {
        auto counter = Meter_->CreateUInt64Counter(name);
        return std::make_shared<TOtelCounter>(std::move(counter), labels);
    }

    std::shared_ptr<IGauge> Gauge(const std::string& name, const TLabels& labels) override {
        auto counter = Meter_->CreateDoubleUpDownCounter(name);
        return std::make_shared<TOtelUpDownCounterGauge>(std::move(counter), labels);
    }

    std::shared_ptr<IHistogram> Histogram(const std::string& name, const std::vector<double>& buckets, const TLabels& labels) override {
        ConfigureHistogramBuckets(name, buckets);
        auto histogram = Meter_->CreateDoubleHistogram(name);
>>>>>>> 1ca4253b5 (fixes and add metric tests)
        return std::make_shared<TOtelHistogram>(std::move(histogram), labels);
    }

private:
<<<<<<< HEAD
    void ConfigureHistogramBuckets(const std::string& name, const std::string& unit, const std::vector<double>& buckets) {
=======
    void ConfigureHistogramBuckets(const std::string& name, const std::vector<double>& buckets) {
>>>>>>> 1ca4253b5 (fixes and add metric tests)
        if (buckets.empty()) {
            return;
        }

<<<<<<< HEAD
        auto* sdkProvider = dynamic_cast<otel_sdk_metrics::MeterProvider*>(MeterProvider_.get());
=======
        auto* sdkProvider = dynamic_cast<sdk::metrics::MeterProvider*>(MeterProvider_.get());
>>>>>>> 1ca4253b5 (fixes and add metric tests)
        if (!sdkProvider) {
            return;
        }

        {
            std::lock_guard lock(HistogramViewsLock_);
            if (!HistogramViews_.insert(name).second) {
                return;
            }
        }

<<<<<<< HEAD
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
=======
        auto selector = std::make_unique<sdk::metrics::InstrumentSelector>(
            sdk::metrics::InstrumentType::kHistogram,
            name,
            ""
        );
        auto meterSelector = std::make_unique<sdk::metrics::MeterSelector>(
            "ydb-cpp-sdk",
            GetSdkSemver(),
            {}
        );

        auto histogramConfig = std::make_shared<sdk::metrics::HistogramAggregationConfig>();
        histogramConfig->boundaries_ = buckets;

        auto view = std::make_unique<sdk::metrics::View>(
            {},
            {},
            sdk::metrics::AggregationType::kHistogram,
>>>>>>> 1ca4253b5 (fixes and add metric tests)
            histogramConfig
        );

        sdkProvider->AddView(std::move(selector), std::move(meterSelector), std::move(view));
    }

<<<<<<< HEAD
    otel_nostd::shared_ptr<otel_metrics::MeterProvider> MeterProvider_;
    otel_nostd::shared_ptr<otel_metrics::Meter> Meter_;
=======
    nostd::shared_ptr<metrics::MeterProvider> MeterProvider_;
    nostd::shared_ptr<metrics::Meter> Meter_;
>>>>>>> 1ca4253b5 (fixes and add metric tests)
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
