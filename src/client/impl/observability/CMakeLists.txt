_ydb_sdk_add_library(impl-observability)

target_link_libraries(impl-observability PUBLIC
  yutil
  client-metrics
)

target_sources(impl-observability PRIVATE
  operation_metrics.cpp
  operation_span.cpp
)

_ydb_sdk_install_targets(TARGETS impl-observability)
