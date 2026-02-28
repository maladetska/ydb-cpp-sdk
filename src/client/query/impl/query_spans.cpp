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

void SafeLogSpanError(const char* message) noexcept {
    try {
        try {
            Cerr << "TQuerySpan: " << message << ": " << CurrentExceptionMessage() << Endl;
            return;
        } catch (...) {
        }
        Cerr << "TQuerySpan: " << message << ": (unknown)" << Endl;
    } catch (...) {
    }
}

} // namespace

TQuerySpan::TQuerySpan(std::shared_ptr<NMetrics::ITracer> tracer, const std::string& operationName, const std::string& endpoint) {
    if (!tracer) {
        return;
    }

    std::string host;
    int port;
    ParseEndpoint(endpoint, host, port);

    try {
        Span_ = tracer->StartSpan("ydb." + operationName, NMetrics::ESpanKind::CLIENT);
        if (!Span_) {
            return;
        }
        Span_->SetAttribute("db.system.name", "ydb");
        Span_->SetAttribute("server.address", host);
        Span_->SetAttribute("server.port", static_cast<int64_t>(port));
    } catch (...) {
        SafeLogSpanError("failed to initialize span");
        Span_.reset();
    }
}

TQuerySpan::~TQuerySpan() noexcept {
    if (Span_) {
        try {
            Span_->End();
        } catch (...) {
            SafeLogSpanError("failed to end span");
        }
    }
}

void TQuerySpan::End(EStatus status) noexcept {
    if (Span_) {
        try {
            Span_->SetAttribute("db.response.status_code", static_cast<int64_t>(status));
            if (status != EStatus::SUCCESS) {
                Span_->SetAttribute("error.type", ToString(status));
            }
            Span_->End();
        } catch (...) {
            SafeLogSpanError("failed to finalize span");
        }
        Span_.reset();
    }
}

} // namespace NYdb::NQuery
