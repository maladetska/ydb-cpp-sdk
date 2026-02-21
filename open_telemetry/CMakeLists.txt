_ydb_sdk_add_library(open_telemetry)

target_sources(open_telemetry PRIVATE
    src/otel.cpp
)

target_include_directories(open_telemetry PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(open_telemetry PUBLIC
    client-metrics
    opentelemetry-cpp::api
    opentelemetry-cpp::metrics
    opentelemetry-cpp::trace
)

_ydb_sdk_make_client_component(OpenTelemetry open_telemetry)
_ydb_sdk_install_headers(${CMAKE_INSTALL_INCLUDEDIR} DIRECTORY include/)
