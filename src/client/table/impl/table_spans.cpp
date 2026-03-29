#include "table_spans.h"

#include <util/string/cast.h>

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

} // namespace

TTableSpan::TTableSpan(std::shared_ptr<NMetrics::ITracer> tracer, const std::string& operationName, const std::string& endpoint) {
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
        Span_.reset();
    }
}

TTableSpan::~TTableSpan() noexcept {
    if (Span_) {
        try {
            Span_->End();
        } catch (...) {
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
        }
        Span_.reset();
    }
}

} // namespace NYdb::NTable
