#include <ydb-cpp-sdk/open_telemetry/trace.h>

#include <opentelemetry/common/attribute_value.h>
<<<<<<< HEAD
#include <opentelemetry/trace/scope.h>
=======
>>>>>>> 1ca4253b5 (fixes and add metric tests)
#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/trace/tracer_provider.h>

namespace NYdb::inline V3::NMetrics {

namespace {

<<<<<<< HEAD
namespace otel_trace = opentelemetry::trace;
namespace otel_nostd = opentelemetry::nostd;
namespace otel_common = opentelemetry::common;

otel_trace::SpanKind MapSpanKind(ESpanKind kind) {
    switch (kind) {
        case ESpanKind::INTERNAL: return otel_trace::SpanKind::kInternal;
        case ESpanKind::SERVER:   return otel_trace::SpanKind::kServer;
        case ESpanKind::CLIENT:   return otel_trace::SpanKind::kClient;
        case ESpanKind::PRODUCER: return otel_trace::SpanKind::kProducer;
        case ESpanKind::CONSUMER: return otel_trace::SpanKind::kConsumer;
    }
    return otel_trace::SpanKind::kInternal;
}

class TOtelScope : public IScope {
public:
    TOtelScope(otel_nostd::shared_ptr<otel_trace::Span> span)
        : Scope_(std::move(span))
    {}

private:
    otel_trace::Scope Scope_;
};

class TOtelSpan : public ISpan {
public:
    TOtelSpan(otel_nostd::shared_ptr<otel_trace::Span> span)
=======
using namespace opentelemetry;

trace::SpanKind MapSpanKind(ESpanKind kind) {
    switch (kind) {
        case ESpanKind::INTERNAL: return trace::SpanKind::kInternal;
        case ESpanKind::SERVER:   return trace::SpanKind::kServer;
        case ESpanKind::CLIENT:   return trace::SpanKind::kClient;
        case ESpanKind::PRODUCER: return trace::SpanKind::kProducer;
        case ESpanKind::CONSUMER: return trace::SpanKind::kConsumer;
    }
    return trace::SpanKind::kInternal;
}

class TOtelSpan : public ISpan {
public:
    TOtelSpan(nostd::shared_ptr<trace::Span> span)
>>>>>>> 1ca4253b5 (fixes and add metric tests)
        : Span_(std::move(span))
    {}

    void End() override {
        Span_->End();
    }

    void SetAttribute(const std::string& key, const std::string& value) override {
        Span_->SetAttribute(key, value);
    }

    void SetAttribute(const std::string& key, int64_t value) override {
        Span_->SetAttribute(key, value);
    }

    void AddEvent(const std::string& name, const std::map<std::string, std::string>& attributes) override {
        if (attributes.empty()) {
            Span_->AddEvent(name);
        } else {
<<<<<<< HEAD
            std::vector<std::pair<otel_nostd::string_view, otel_common::AttributeValue>> attrs;
            attrs.reserve(attributes.size());
            for (const auto& [k, v] : attributes) {
                attrs.emplace_back(otel_nostd::string_view(k), otel_common::AttributeValue(otel_nostd::string_view(v)));
=======
            std::vector<std::pair<nostd::string_view, common::AttributeValue>> attrs;
            attrs.reserve(attributes.size());
            for (const auto& [k, v] : attributes) {
                attrs.emplace_back(nostd::string_view(k), common::AttributeValue(nostd::string_view(v)));
>>>>>>> 1ca4253b5 (fixes and add metric tests)
            }
            Span_->AddEvent(name, attrs);
        }
    }

<<<<<<< HEAD
    std::unique_ptr<IScope> Activate() override {
        return std::make_unique<TOtelScope>(Span_);
    }

private:
    otel_nostd::shared_ptr<otel_trace::Span> Span_;
=======
private:
    nostd::shared_ptr<trace::Span> Span_;
>>>>>>> 1ca4253b5 (fixes and add metric tests)
};

class TOtelTracer : public ITracer {
public:
<<<<<<< HEAD
    TOtelTracer(otel_nostd::shared_ptr<otel_trace::Tracer> tracer)
=======
    TOtelTracer(nostd::shared_ptr<trace::Tracer> tracer)
>>>>>>> 1ca4253b5 (fixes and add metric tests)
        : Tracer_(std::move(tracer))
    {}

    std::shared_ptr<ISpan> StartSpan(const std::string& name, ESpanKind kind) override {
<<<<<<< HEAD
        otel_trace::StartSpanOptions options;
=======
        trace::StartSpanOptions options;
>>>>>>> 1ca4253b5 (fixes and add metric tests)
        options.kind = MapSpanKind(kind);
        return std::make_shared<TOtelSpan>(Tracer_->StartSpan(name, options));
    }

private:
<<<<<<< HEAD
    otel_nostd::shared_ptr<otel_trace::Tracer> Tracer_;
=======
    nostd::shared_ptr<trace::Tracer> Tracer_;
>>>>>>> 1ca4253b5 (fixes and add metric tests)
};

class TOtelTraceProvider : public ITraceProvider {
public:
<<<<<<< HEAD
    TOtelTraceProvider(otel_nostd::shared_ptr<otel_trace::TracerProvider> tracerProvider)
=======
    TOtelTraceProvider(nostd::shared_ptr<trace::TracerProvider> tracerProvider)
>>>>>>> 1ca4253b5 (fixes and add metric tests)
        : TracerProvider_(std::move(tracerProvider))
    {}

    std::shared_ptr<ITracer> GetTracer(const std::string& name) override {
        return std::make_shared<TOtelTracer>(TracerProvider_->GetTracer(name));
    }

private:
<<<<<<< HEAD
    otel_nostd::shared_ptr<otel_trace::TracerProvider> TracerProvider_;
=======
    nostd::shared_ptr<trace::TracerProvider> TracerProvider_;
>>>>>>> 1ca4253b5 (fixes and add metric tests)
};

} // namespace

std::shared_ptr<ITraceProvider> CreateOtelTraceProvider(
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider)
{
    return std::make_shared<TOtelTraceProvider>(std::move(tracerProvider));
}

} // namespace NYdb::NMetrics
