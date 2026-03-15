<<<<<<< HEAD
<<<<<<< HEAD
add_subdirectory(metrics)
add_subdirectory(trace)
=======
if (YDB_SDK_ENABLE_OTEL_METRICS OR YDB_SDK_ENABLE_OTEL_TRACE)
  add_subdirectory(open_telemetry EXCLUDE_FROM_ALL)
endif()
>>>>>>> 1b2bf4fa5 (fixes)
=======
add_subdirectory(metrics)
add_subdirectory(trace)
>>>>>>> 1ca4253b5 (fixes and add metric tests)
