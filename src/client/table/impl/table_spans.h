#pragma once

#include <ydb-cpp-sdk/client/trace/trace.h>
#include <ydb-cpp-sdk/client/types/status_codes.h>
#include <ydb-cpp-sdk/client/types/status/status.h>

#include <library/cpp/logger/log.h>

#include <map>
#include <memory>
#include <string>

namespace NYdb::inline V3::NTable {

class TTableSpan {
public:
    TTableSpan(std::shared_ptr<NMetrics::ITracer> tracer
        , const std::string& operationName
        , const std::string& endpoint
        , const TLog& log
    );
    ~TTableSpan() noexcept;

    void End(EStatus status) noexcept;

private:
    TLog Log_;
    std::shared_ptr<NMetrics::ISpan> Span_;
};

} // namespace NYdb::NTable
