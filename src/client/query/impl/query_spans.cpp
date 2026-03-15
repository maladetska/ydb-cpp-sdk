#include "query_spans.h"

#include <util/string/cast.h>

namespace NYdb::inline V3::NQuery {

namespace {

constexpr int DefaultGrpcPort = 2135;

void ParseEndpoint(const std::string& endpoint, std::string& host, int& port) {
    port = DefaultGrpcPort;

    if (endpoint.empty()) {
        host = endpoint;
        return;
    }

    // IPv6 bracket notation: [addr]:port
    if (endpoint.front() == '[') {
        auto bracketEnd = endpoint.find(']');
        if (bracketEnd != std::string::npos) {
            host = endpoint.substr(1, bracketEnd - 1);
            if (bracketEnd + 2 < endpoint.size() && endpoint[bracketEnd + 1] == ':') {
                try {
                    port = std::stoi(endpoint.substr(bracketEnd + 2));
                } catch (...) {}
            }
            return;
        }
    }

    auto pos = endpoint.rfind(':');
    if (pos != std::string::npos) {
        host = endpoint.substr(0, pos);
        try {
            port = std::stoi(endpoint.substr(pos + 1));
        } catch (...) {}
    } else {
        host = endpoint;
    }
}

void SafeLogSpanError(const char* message) noexcept {
    try {
        try {
            std::cerr << "TQuerySpan: " << message << ": " << CurrentExceptionMessage() << std::endl;
            return;
        } catch (...) {
        }
        std::cerr << "TQuerySpan: " << message << ": (unknown)" << std::endl;
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

void TQuerySpan::SetPeerEndpoint(const std::string& endpoint) noexcept {
    if (!Span_ || endpoint.empty()) {
        return;
    }
    try {
        std::string host;
        int port;
        ParseEndpoint(endpoint, host, port);
        Span_->SetAttribute("network.peer.address", host);
        Span_->SetAttribute("network.peer.port", static_cast<int64_t>(port));
    } catch (...) {
        SafeLogSpanError("failed to set peer endpoint");
    }
}

void TQuerySpan::SetQueryText(const std::string& query) noexcept {
    if (!Span_ || query.empty()) {
        return;
    }
    try {
        Span_->SetAttribute("db.query.text", query);
    } catch (...) {
        SafeLogSpanError("failed to set query text");
    }
}

void TQuerySpan::AddEvent(const std::string& name, const std::map<std::string, std::string>& attributes) noexcept {
    if (!Span_) {
        return;
    }
    try {
        Span_->AddEvent(name, attributes);
    } catch (...) {
        SafeLogSpanError("failed to add event");
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
