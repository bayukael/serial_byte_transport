cmake_minimum_required(VERSION 3.13)
project(pendarlab_lib_comm_serial_byte_transport)

set(PROJECT_VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(NAMESPACE pendarlab)
set(PACKAGE_FILENAME serial_byte_transport)
set(PACKAGE_NAME PendarlabSerialByteTransport)

find_package(PendarlabByteTransport REQUIRED)
find_package(PendarlabSerialDevice REQUIRED)
find_package(GTest CONFIG REQUIRED)

add_library(SerialByteTransport SHARED
  src/SerialByteTransport.cpp
  src/SerialByteTransportConfig.cpp
  src/SerialByteTransportDefinition.cpp
)
target_include_directories(SerialByteTransport PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<INSTALL_INTERFACE:include> 
)
target_link_libraries(SerialByteTransport 
  PUBLIC pendarlab::ByteTransport
  PUBLIC pendarlab::SerialDevice
)
add_library(${NAMESPACE}::SerialByteTransport ALIAS SerialByteTransport)
set_target_properties(SerialByteTransport PROPERTIES OUTPUT_NAME pendarlab-serial_byte_transport)


##############################################################################
######################     Define Files to Install     #######################
##############################################################################

set(TARGET_EXPORT_NAME SerialByteTransportTargets)
set(TARGET_EXPORT_FILENAME serial_byte_transport_targets)
set(PACKAGE_DIRECTORY_DESTINATION lib/cmake/${PACKAGE_NAME})

install(
	TARGETS SerialByteTransport
	EXPORT ${TARGET_EXPORT_NAME}
)
install(
	DIRECTORY include/
	DESTINATION include
)
install(
	EXPORT ${TARGET_EXPORT_NAME}
	FILE ${TARGET_EXPORT_FILENAME}.cmake
	NAMESPACE ${NAMESPACE}::
	DESTINATION ${PACKAGE_DIRECTORY_DESTINATION}
)

##############################################################################
##########################     CMake Packaging     ###########################
##############################################################################

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PACKAGE_FILENAME}-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}Config.cmake
    INSTALL_DESTINATION ${PACKAGE_DIRECTORY_DESTINATION}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}ConfigVersion.cmake
    DESTINATION ${PACKAGE_DIRECTORY_DESTINATION}
)

##################################################################################################################
################################################  BUILDING TEST  #################################################
##################################################################################################################
add_executable(serial_byte_transport_config_test test/serial_byte_transport_config_test.cpp)
target_include_directories(serial_byte_transport_config_test PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(serial_byte_transport_config_test
  PRIVATE SerialByteTransport
  PRIVATE gtest
)

add_executable(serial_byte_transport_test test/serial_byte_transport_test.cpp)
target_include_directories(serial_byte_transport_test PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(serial_byte_transport_test
  PRIVATE SerialByteTransport
  PRIVATE gtest
)

add_executable(serial_byte_transport_integration_test test/serial_byte_transport_integration_test.cpp)
target_include_directories(serial_byte_transport_integration_test PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(serial_byte_transport_integration_test
  PRIVATE SerialByteTransport
  PRIVATE gtest
)
