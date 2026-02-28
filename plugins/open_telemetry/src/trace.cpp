#include <ydb-cpp-sdk/open_telemetry/trace.h>

#include <opentelemetry/trace/tracer.h>

namespace NYdb::inline V3::NMetrics {

namespace {

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

private:
    nostd::shared_ptr<trace::Span> Span_;
};

class TOtelTracer : public ITracer {
public:
    TOtelTracer(nostd::shared_ptr<trace::Tracer> tracer)
        : Tracer_(std::move(tracer))
    {}

    std::shared_ptr<ISpan> StartSpan(const std::string& name, ESpanKind kind) override {
        trace::StartSpanOptions options;
        options.kind = MapSpanKind(kind);
        return std::make_shared<TOtelSpan>(Tracer_->StartSpan(name, options));
    }

private:
    nostd::shared_ptr<trace::Tracer> Tracer_;
};

} // namespace

TOtelTraceProvider::TOtelTraceProvider(nostd::shared_ptr<trace::TracerProvider> tracerProvider)
    : TracerProvider_(std::move(tracerProvider))
{}

std::shared_ptr<ITracer> TOtelTraceProvider::GetTracer(const std::string& name) {
    return std::make_shared<TOtelTracer>(TracerProvider_->GetTracer(name));
}

} // namespace NYdb::NMetrics
