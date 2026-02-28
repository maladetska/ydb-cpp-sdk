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
<<<<<<< HEAD
        Span_ = tracer->StartSpan(operationName, NMetrics::ESpanKind::CLIENT);
        if (!Span_) {
            return;
        }
        Span_->SetAttribute("db.system.name", "other_sql");
        Span_->SetAttribute("db.operation.name", operationName);
=======
        Span_ = tracer->StartSpan("ydb." + operationName, NMetrics::ESpanKind::CLIENT);
        if (!Span_) {
            return;
        }
        Span_->SetAttribute("db.system.name", "ydb");
>>>>>>> 1b2bf4fa5 (fixes)
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

<<<<<<< HEAD
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

std::unique_ptr<NMetrics::IScope> TQuerySpan::Activate() noexcept {
    if (!Span_) {
        return nullptr;
    }
    try {
        return Span_->Activate();
    } catch (...) {
        SafeLogSpanError("failed to activate span");
        return nullptr;
    }
}

void TQuerySpan::End(EStatus status) noexcept {
    if (Span_) {
        try {
            Span_->SetAttribute("db.response.status_code", ToString(status));
=======
void TQuerySpan::End(EStatus status) noexcept {
    if (Span_) {
        try {
            Span_->SetAttribute("db.response.status_code", static_cast<int64_t>(status));
>>>>>>> 1b2bf4fa5 (fixes)
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
