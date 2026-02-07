#include "query_spans.h"

#include <util/string/cast.h>

namespace NYdb::inline V3::NQuery {

namespace {

void ParseEndpoint(const std::string& endpoint, std::string& host, int& port) {
    auto pos = endpoint.find(':');
    if (pos != std::string::npos) {
        host = endpoint.substr(0, pos);
        try {
            port = std::stoi(endpoint.substr(pos + 1));
        } catch (...) {
            port = 2135;
        }
    } else {
        host = endpoint;
        port = 2135;
    }
}

} // namespace

TQuerySpan::TQuerySpan(std::shared_ptr<NMetrics::ITracer> tracer, const std::string& operationName, const std::string& endpoint) {
    if (!tracer) return;

    std::string host;
    int port;
    ParseEndpoint(endpoint, host, port);

    Span_ = tracer->StartSpan("ydb." + operationName, NMetrics::ESpanKind::CLIENT);
    Span_->SetAttribute("db.system.name", "ydb");
    Span_->SetAttribute("server.address", host);
    Span_->SetAttribute("server.port", static_cast<int64_t>(port));
}

TQuerySpan::~TQuerySpan() {
    if (Span_) {
        Span_->End();
    }
}

void TQuerySpan::End(EStatus status) {
    if (Span_) {
        Span_->SetAttribute("db.response.status_code", static_cast<int64_t>(status));
        if (status != EStatus::SUCCESS) {
            Span_->SetAttribute("error.type", ToString(status));
        }
        Span_->End();
        Span_.reset();
    }
}

} // namespace NYdb::NQuery
