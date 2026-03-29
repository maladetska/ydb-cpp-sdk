#pragma once

#include <src/client/impl/observability/client_metrics.h>

namespace NYdb::inline V3::NQuery {

class TQueryMetrics : public NObservability::TClientMetrics {
public:
    TQueryMetrics(std::shared_ptr<NMetrics::IMetricRegistry> registry, const std::string& operationName)
        : TClientMetrics(std::move(registry), operationName)
    {}
};

} // namespace NYdb::NQuery
