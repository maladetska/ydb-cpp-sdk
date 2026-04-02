_ydb_sdk_add_library(client-impl-ydb_stats)

target_link_libraries(client-impl-ydb_stats PUBLIC
  yutil
  grpc-client
  monlib-metrics
  client-metrics
  impl-internal-common
)

target_sources(client-impl-ydb_stats PRIVATE
  stats.cpp
)

_ydb_sdk_install_targets(TARGETS client-impl-ydb_stats)
