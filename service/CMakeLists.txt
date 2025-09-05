cmake_minimum_required(VERSION 3.16)
project(tcp_like_udp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(tcp-like)
add_subdirectory(nat-traversal)
add_subdirectory(store)
add_subdirectory(p2p)


