_ydb_sdk_add_library(client-metrics)

target_sources(client-metrics PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
  metrics.cpp
=======
    metrics.cpp
>>>>>>> 1b2bf4fa5 (fixes)
=======
  metrics.cpp
>>>>>>> 1ca4253b5 (fixes and add metric tests)
)

_ydb_sdk_make_client_component(Metrics client-metrics)
