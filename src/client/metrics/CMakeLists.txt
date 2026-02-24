_ydb_sdk_add_library(client-metrics)

target_sources(client-metrics PRIVATE
    metrics.cpp
)

target_include_directories(client-metrics PUBLIC
    $<BUILD_INTERFACE:${YDB_SDK_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(client-metrics PUBLIC
    client-extension_common
)

_ydb_sdk_make_client_component(Metrics client-metrics)
