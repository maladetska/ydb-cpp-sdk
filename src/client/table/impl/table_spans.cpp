#include "table_spans.h"

#include <src/client/topic/common/log_lazy.h>

#include <util/string/cast.h>

#include <exception>

namespace NYdb::inline V3::NTable {

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

void SafeLogSpanError(TLog& log, const char* message) noexcept {
    try {
        try {
            std::rethrow_exception(std::current_exception());
        } catch (const std::exception& e) {
            LOG_LAZY(log, TLOG_ERR, std::string("TTableSpan: ") + message + ": " + e.what());
            return;
        } catch (...) {
        }
        LOG_LAZY(log, TLOG_ERR, std::string("TTableSpan: ") + message + ": (unknown)");
    } catch (...) {
    }
}

} // namespace

TTableSpan::TTableSpan(std::shared_ptr<NMetrics::ITracer> tracer
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
        Span_ = tracer->StartSpan(operationName, NMetrics::ESpanKind::CLIENT);
        if (!Span_) {
            return;
        }
        Span_->SetAttribute("db.system.name", "other_sql");
        Span_->SetAttribute("db.operation.name", operationName);
        Span_->SetAttribute("server.address", host);
        Span_->SetAttribute("server.port", static_cast<int64_t>(port));
    } catch (...) {
        SafeLogSpanError(Log_, "failed to initialize span");
        Span_.reset();
    }
}

TTableSpan::~TTableSpan() noexcept {
    if (Span_) {
        try {
            Span_->End();
        } catch (...) {
            SafeLogSpanError(Log_, "failed to end span");
        }
    }
}

void TTableSpan::End(EStatus status) noexcept {
    if (Span_) {
        try {
            Span_->SetAttribute("db.response.status_code", ToString(status));
            if (status != EStatus::SUCCESS) {
                Span_->SetAttribute("error.type", ToString(status));
            }
            Span_->End();
        } catch (...) {
            SafeLogSpanError(Log_, "failed to finalize span");
        }
        Span_.reset();
    }
}

} // namespace NYdb::NTable
