if (YDB_SDK_ENABLE_OTEL_METRICS OR YDB_SDK_ENABLE_OTEL_TRACE)
  add_subdirectory(open_telemetry EXCLUDE_FROM_ALL)
endif()
