cmake_minimum_required(VERSION 3.28)
project(CPlusSharpQueue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(protobuf CONFIG REQUIRED)

add_subdirectory(proto)
add_subdirectory(agent)
