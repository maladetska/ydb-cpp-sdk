_ydb_sdk_add_library(client-metrics)

target_sources(client-metrics PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
  metrics.cpp
=======
    metrics.cpp
<<<<<<< HEAD
>>>>>>> 1b2bf4fa5 (fixes)
=======
  metrics.cpp
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
>>>>>>> a979e6bda (fixes)
=======
  metrics.cpp
>>>>>>> dcae6d69e (fixes and add metric tests)
)

_ydb_sdk_make_client_component(Metrics client-metrics)
