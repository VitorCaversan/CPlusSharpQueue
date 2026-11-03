add_executable(agent src/main.cpp src/service.cpp src/worker.cpp)
target_link_libraries(agent PRIVATE telemetry_proto)
target_compile_definitions(agent PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(agent PRIVATE /W4 /permissive- /utf-8)
