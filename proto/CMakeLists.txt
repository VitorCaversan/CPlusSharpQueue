add_library(telemetry_proto STATIC telemetry.proto)
target_link_libraries(telemetry_proto PUBLIC protobuf::libprotobuf)
target_include_directories(telemetry_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate(TARGET telemetry_proto LANGUAGE cpp)
