#include "operation_span.h"

#include <src/client/impl/internal/common/log_lazy.h>

#include <util/string/cast.h>

#include <exception>

namespace NYdb::inline V3::NObservability {

namespace {

constexpr int DefaultGrpcPort = 2135;

void ParseEndpoint(const std::string& endpoint, std::string& host, int& port) {
    port = DefaultGrpcPort;

    if (endpoint.empty()) {
        host = endpoint;
        return;
    }

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

void SafeLogSpanError(TLog& log, const char* message, std::exception_ptr exception) noexcept {
    try {
        if (!exception) {
            LOG_LAZY(log, TLOG_ERR, std::string("TOperationSpan: ") + message + ": (no active exception)");
            return;
        }
        try {
            std::rethrow_exception(exception);
        } catch (const std::exception& e) {
            LOG_LAZY(log, TLOG_ERR, std::string("TOperationSpan: ") + message + ": " + e.what());
            return;
        } catch (...) {
        }
        LOG_LAZY(log, TLOG_ERR, std::string("TOperationSpan: ") + message + ": (unknown)");
    } catch (...) {
    }
}

} // namespace

TOperationSpan::TOperationSpan(std::shared_ptr<NTrace::ITracer> tracer
    , const std::string& operationName
    , const std::string& endpoint
    , const TLog& log
) : Log_(log) {
    if (!tracer) {
        return;
    }

    std::string host;
    int port;
    ParseEndpoint(endpoint, host, port);

    try {
        Span_ = tracer->StartSpan(operationName, NTrace::ESpanKind::CLIENT);
        if (!Span_) {
            return;
        }
        Span_->SetAttribute("db.system.name", "other_sql");
        Span_->SetAttribute("db.operation.name", operationName);
        Span_->SetAttribute("server.address", host);
        Span_->SetAttribute("server.port", static_cast<int64_t>(port));
    } catch (...) {
        SafeLogSpanError(Log_, "failed to initialize span", std::current_exception());
        Span_.reset();
    }
}

TOperationSpan::~TOperationSpan() noexcept {
    if (Span_) {
        try {
            Span_->End();
        } catch (...) {
            SafeLogSpanError(Log_, "failed to end span", std::current_exception());
        }
    }
}

void TOperationSpan::SetPeerEndpoint(const std::string& endpoint) noexcept {
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
        SafeLogSpanError(Log_, "failed to set peer endpoint", std::current_exception());
    }
}

void TOperationSpan::AddEvent(const std::string& name, const std::map<std::string, std::string>& attributes) noexcept {
    if (!Span_) {
        return;
    }
    try {
        Span_->AddEvent(name, attributes);
    } catch (...) {
        SafeLogSpanError(Log_, "failed to add event", std::current_exception());
    }
}

void TOperationSpan::End(EStatus status) noexcept {
    if (Span_) {
        try {
            Span_->SetAttribute("db.response.status_code", static_cast<int64_t>(status));
            if (status != EStatus::SUCCESS) {
                Span_->SetAttribute("error.type", ToString(status));
            }
            Span_->End();
        } catch (...) {
            SafeLogSpanError(Log_, "failed to finalize span", std::current_exception());
        }
        Span_.reset();
    }
}

} // namespace NYdb::NObservability
