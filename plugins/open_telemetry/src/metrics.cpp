#include <ydb-cpp-sdk/open_telemetry/metrics.h>
#include <ydb-cpp-sdk/client/resources/ydb_resources.h>

#include <opentelemetry/common/key_value_iterable_view.h>
#include <opentelemetry/context/runtime_context.h>
#include <opentelemetry/metrics/meter.h>
#include <opentelemetry/metrics/sync_instruments.h>
#include <opentelemetry/sdk/metrics/meter_context.h>
#include <opentelemetry/sdk/metrics/meter_provider.h>

namespace NYdb::inline V3::NMetrics {

namespace {

using namespace opentelemetry;

common::KeyValueIterableView<TLabels> MakeAttributes(const TLabels& labels) {
    return common::KeyValueIterableView<TLabels>(labels);
}

class TOtelCounter : public ICounter {
public:
    TOtelCounter(nostd::shared_ptr<metrics::Counter<uint64_t>> counter, const TLabels& labels)
        : Counter_(std::move(counter))
        , Labels_(labels)
    {}

    void Inc() override {
        Counter_->Add(1, MakeAttributes(Labels_), context::RuntimeContext::GetCurrent());
    }

private:
    nostd::shared_ptr<metrics::Counter<uint64_t>> Counter_;
    TLabels Labels_;
};

class TOtelUpDownCounterGauge : public IGauge {
public:
    TOtelUpDownCounterGauge(nostd::shared_ptr<metrics::UpDownCounter<double>> counter, const TLabels& labels)
        : Counter_(std::move(counter))
        , Labels_(labels)
    {}

    void Add(double delta) override {
        Counter_->Add(delta, MakeAttributes(Labels_), context::RuntimeContext::GetCurrent());
        Value_ += delta;
    }

    void Set(double value) override {
        Counter_->Add(value - Value_, MakeAttributes(Labels_), context::RuntimeContext::GetCurrent());
        Value_ = value;
    }

private:
    nostd::shared_ptr<metrics::UpDownCounter<double>> Counter_;
    TLabels Labels_;
    double Value_ = 0;
};

class TOtelHistogram : public IHistogram {
public:
    TOtelHistogram(nostd::shared_ptr<metrics::Histogram<double>> histogram, const TLabels& labels)
        : Histogram_(std::move(histogram))
        , Labels_(labels)
    {}

    void Record(double value) override {
        Histogram_->Record(value, MakeAttributes(Labels_), context::RuntimeContext::GetCurrent());
    }

private:
    nostd::shared_ptr<metrics::Histogram<double>> Histogram_;
    TLabels Labels_;
};

} // namespace

TOtelMetricRegistry::TOtelMetricRegistry(nostd::shared_ptr<metrics::MeterProvider> meterProvider)
    : MeterProvider_(std::move(meterProvider))
    , Meter_(MeterProvider_->GetMeter("ydb-cpp-sdk", GetSdkSemver()))
{}

void TOtelMetricRegistry::ConfigureHistogramBuckets(const std::string& name, const std::vector<double>& buckets) {
    if (buckets.empty()) {
        return;
    }

    auto* sdkProvider = dynamic_cast<sdk::metrics::MeterProvider*>(MeterProvider_.get());
    if (!sdkProvider) {
        return;
    }

    {
        std::lock_guard lock(HistogramViewsLock_);
        if (!HistogramViews_.insert(name).second) {
            return;
        }
    }

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
        histogramConfig
    );

    sdkProvider->AddView(std::move(selector), std::move(meterSelector), std::move(view));
}

std::shared_ptr<ICounter> TOtelMetricRegistry::Counter(const std::string& name, const TLabels& labels) {
    auto counter = Meter_->CreateUInt64Counter(name);
    return std::make_shared<TOtelCounter>(std::move(counter), labels);
}

std::shared_ptr<IGauge> TOtelMetricRegistry::Gauge(const std::string& name, const TLabels& labels) {
    auto counter = Meter_->CreateDoubleUpDownCounter(name);
    return std::make_shared<TOtelUpDownCounterGauge>(std::move(counter), labels);
}

std::shared_ptr<IHistogram> TOtelMetricRegistry::Histogram(const std::string& name, const std::vector<double>& buckets, const TLabels& labels) {
    ConfigureHistogramBuckets(name, buckets);
    auto histogram = Meter_->CreateDoubleHistogram(name);
    return std::make_shared<TOtelHistogram>(std::move(histogram), labels);
}

} // namespace NYdb::NMetrics
