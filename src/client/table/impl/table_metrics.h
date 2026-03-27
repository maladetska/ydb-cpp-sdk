#pragma once

#include <src/client/impl/observability/client_metrics.h>

namespace NYdb::inline V3::NTable {

class TTableMetrics : public NObservability::TClientMetrics {
public:
    TTableMetrics(std::shared_ptr<NMetrics::IMetricRegistry> registry, const std::string& operationName)
        : TClientMetrics(std::move(registry), "ydb.table", operationName)
    {}
};

} // namespace NYdb::NTable
