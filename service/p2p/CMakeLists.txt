cmake_minimum_required(VERSION 3.16)

project(p2p LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(OpenSSL QUIET)

add_library(p2p
    src/peer.cpp
    src/router.cpp
    src/dht.cpp
    src/data_api.cpp
    src/protocol.cpp
    src/node.cpp
)

target_include_directories(p2p
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(p2p
    PUBLIC
        tcp_like_udp
        nat_traversal
        store
        OpenSSL::Crypto
)

if(PROJECT_IS_TOP_LEVEL)
    add_executable(p2p_demo examples/main.cpp)
    target_link_libraries(p2p_demo PRIVATE p2p)
endif()


