cmake_minimum_required(VERSION 3.16)

project(repeater_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(repeater
    src/server.cpp
    src/codec.cpp
    src/registry.cpp
)

target_include_directories(repeater
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)


