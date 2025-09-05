cmake_minimum_required(VERSION 3.16)

add_executable(service_main main.cpp)
target_link_libraries(service_main PRIVATE p2p repeater)
target_include_directories(service_main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../p2p/include)

