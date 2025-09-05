cmake_minimum_required(VERSION 3.14)

project(store LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(store
    src/store.cpp
)

target_include_directories(store
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(PROJECT_IS_TOP_LEVEL)
    add_executable(store_demo examples/simple_demo.cpp)
    target_link_libraries(store_demo PRIVATE store)
endif()


