cmake_minimum_required(VERSION 3.13)
project(pendarlab_lib_comm_serial_byte_transport)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

find_package(GTest CONFIG REQUIRED)

add_subdirectory(external/byte_transport)
add_subdirectory(external/serial_device)

add_library(pendarlab_comm_serial_byte_transport SHARED
  src/SerialByteTransport.cpp
  src/SerialByteTransportConfig.cpp
)
target_include_directories(pendarlab_comm_serial_byte_transport PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/byte_transport/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/serial_device/include>
)
target_link_libraries(pendarlab_comm_serial_byte_transport
  PUBLIC pendarlab_comm_byte_transport
  PUBLIC pendarlab_serial_device
)

##################################################################################################################
################################################  BUILDING TEST  #################################################
##################################################################################################################
add_executable(serial_byte_transport_config_test test/serial_byte_transport_config_test.cpp)
target_include_directories(serial_byte_transport_config_test PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
)
target_link_libraries(serial_byte_transport_config_test
  PRIVATE pendarlab_comm_serial_byte_transport
  PRIVATE gtest
)