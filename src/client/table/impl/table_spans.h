#pragma once

#include <src/client/impl/observability/operation_span.h>

namespace NYdb::inline V3::NTable {

class TTableSpan : public NObservability::TOperationSpan {
public:
    TTableSpan(std::shared_ptr<NTrace::ITracer> tracer
        , const std::string& operationName
        , const std::string& endpoint
        , const TLog& log
    ) : TOperationSpan(std::move(tracer), operationName, endpoint, log)
    {}
};

} // namespace NYdb::NTable
