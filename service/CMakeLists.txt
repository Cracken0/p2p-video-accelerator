cmake_minimum_required(VERSION 3.16)
project(tcp_like_udp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(tcp-like)
add_subdirectory(nat-traversal)
add_subdirectory(store)
add_subdirectory(p2p)

# 引入 repeater C++ 库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../repeater/cpp ${CMAKE_CURRENT_BINARY_DIR}/repeater_cpp)

# 可执行入口
add_subdirectory(bin)


