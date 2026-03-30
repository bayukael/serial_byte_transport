cmake_minimum_required(VERSION 3.13)
project(pendarlab_lib_comm_serial_byte_transport)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

find_package(GTest CONFIG REQUIRED)

add_library(pendarlab_comm_serial_byte_transport SHARED
  src/SerialByteTransport.cpp
)
target_include_directories(pendarlab_comm_serial_byte_transport PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)