#pragma once

#include <src/client/impl/observability/operation_metrics.h>

namespace NYdb::inline V3::NTable {

class TTableMetrics : public NObservability::TOperationMetrics {
public:
    TTableMetrics(std::shared_ptr<NMetrics::IMetricRegistry> registry
        , const std::string& operationName
        , const TLog& log
    ) : TOperationMetrics(std::move(registry), operationName, log)
    {}
};

} // namespace NYdb::NTable
