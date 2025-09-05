cmake_minimum_required(VERSION 3.16)

add_library(nat_traversal
    src/nat_detector.cpp
    src/nat_agent.cpp
)

target_include_directories(nat_traversal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nat_traversal PUBLIC tcp_like_udp)
target_compile_features(nat_traversal PUBLIC cxx_std_17)







