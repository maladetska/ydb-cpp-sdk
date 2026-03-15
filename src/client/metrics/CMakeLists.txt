_ydb_sdk_add_library(client-metrics)

target_sources(client-metrics PRIVATE
  metrics.cpp
)

_ydb_sdk_make_client_component(Metrics client-metrics)
