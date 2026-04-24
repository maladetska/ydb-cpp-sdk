_ydb_sdk_add_library(client-trace)

target_sources(client-trace PRIVATE
  trace.cpp
)

_ydb_sdk_make_client_component(Trace client-trace)