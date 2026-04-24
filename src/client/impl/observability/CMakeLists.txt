_ydb_sdk_add_library(impl-observability)

target_link_libraries(impl-observability PUBLIC
  yutil
  client-metrics
  client-impl-ydb_stats
)

target_sources(impl-observability PRIVATE
  metrics.cpp
  observation.cpp
  span.cpp
)

_ydb_sdk_install_targets(TARGETS impl-observability)