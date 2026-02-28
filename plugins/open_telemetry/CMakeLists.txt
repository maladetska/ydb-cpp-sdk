if (YDB_SDK_ENABLE_OTEL_METRICS)
  _ydb_sdk_add_library(open_telemetry_metrics)
  target_sources(open_telemetry_metrics PRIVATE
      src/metrics.cpp
  )
  target_include_directories(open_telemetry_metrics PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
  )
  target_link_libraries(open_telemetry_metrics PUBLIC
      client-metrics
      client-resources
      opentelemetry-cpp::api
      opentelemetry-cpp::metrics
  )
  _ydb_sdk_make_client_component(OpenTelemetryMetrics open_telemetry_metrics)
endif()

if (YDB_SDK_ENABLE_OTEL_TRACE)
  _ydb_sdk_add_library(open_telemetry_trace)
  target_sources(open_telemetry_trace PRIVATE
      src/trace.cpp
  )
  target_include_directories(open_telemetry_trace PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
  )
  target_link_libraries(open_telemetry_trace PUBLIC
      client-metrics
      opentelemetry-cpp::api
      opentelemetry-cpp::trace
  )
  _ydb_sdk_make_client_component(OpenTelemetryTrace open_telemetry_trace)
endif()

_ydb_sdk_install_headers(${CMAKE_INSTALL_INCLUDEDIR} DIRECTORY include/)
