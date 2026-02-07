_ydb_sdk_add_library(open_telemetry)

target_sources(open_telemetry PRIVATE
    otel.cpp
)

target_include_directories(open_telemetry PUBLIC
    $<BUILD_INTERFACE:${YDB_SDK_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(open_telemetry PUBLIC
    client-metrics
    opentelemetry-cpp::api
    opentelemetry-cpp::metrics
    opentelemetry-cpp::trace
)
