add_executable(agent src/main.cpp)
target_link_libraries(agent PRIVATE telemetry_proto)
target_compile_options(agent PRIVATE /W4 /permissive- /utf-8)
