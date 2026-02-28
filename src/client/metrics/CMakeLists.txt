_ydb_sdk_add_library(client-metrics)

target_sources(client-metrics PRIVATE
<<<<<<< HEAD
  metrics.cpp
=======
    metrics.cpp
>>>>>>> 1b2bf4fa5 (fixes)
)

_ydb_sdk_make_client_component(Metrics client-metrics)
