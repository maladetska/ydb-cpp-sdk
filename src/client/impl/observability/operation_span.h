#pragma once

#include <ydb-cpp-sdk/client/trace/trace.h>
#include <ydb-cpp-sdk/client/types/status_codes.h>
#include <ydb-cpp-sdk/client/types/status/status.h>

#include <library/cpp/logger/log.h>

#include <map>
#include <memory>
#include <string>

namespace NYdb::inline V3::NObservability {

class TOperationSpan {
public:
    TOperationSpan(std::shared_ptr<NTrace::ITracer> tracer, const std::string& operationName,
        const std::string& endpoint, const TLog& log);
    ~TOperationSpan() noexcept;

    void SetPeerEndpoint(const std::string& endpoint) noexcept;
    void AddEvent(const std::string& name, const std::map<std::string, std::string>& attributes = {}) noexcept;
    std::unique_ptr<NTrace::IScope> Activate() noexcept;

    void End(EStatus status) noexcept;

private:
    TLog Log_;
    std::shared_ptr<NTrace::ISpan> Span_;
};

} // namespace NYdb::NObservability
