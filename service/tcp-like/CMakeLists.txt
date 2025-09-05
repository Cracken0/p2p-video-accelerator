cmake_minimum_required(VERSION 3.16)

add_library(tcp_like_udp
    src/udp_socket.cpp
    src/reliable_session.cpp
)

target_include_directories(tcp_like_udp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(tcp_like_udp PUBLIC cxx_std_17)

add_executable(echo_server examples/echo_server.cpp)
target_link_libraries(echo_server PRIVATE tcp_like_udp)

add_executable(echo_client examples/echo_client.cpp)
target_link_libraries(echo_client PRIVATE tcp_like_udp)







