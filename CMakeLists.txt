cmake_minimum_required(VERSION 3.7)

project(pdm VERSION 1.0.0)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

set(SRC_FILES 
	src/PDM.cpp
	src/Gatherer.cpp
	src/Wine/Wine.cpp
	src/Windows/WindowsData.cpp
	src/Windows/D3D11Info.cpp
	src/Windows/VulkanInfo.cpp
	src/MacOS/MacOSData.cpp
)

if(NOT APPLE)
	set(CMAKE_INSTALL_RPATH $ORIGIN)
endif()

add_library(pdm STATIC
	${SRC_FILES}
)
add_library(ccp::pdm ALIAS pdm)

set_property(TARGET pdm PROPERTY CXX_STANDARD 17)
set_property(TARGET pdm PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET pdm PROPERTY CXX_VISIBILITY_PRESET hidden)
set_property(TARGET pdm PROPERTY VISIBILITY_INLINES_HIDDEN YES)

target_include_directories(pdm
  PUBLIC
    $<INSTALL_INTERFACE:include/pdm>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/pdm)

install(TARGETS pdm
  EXPORT pdm-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
set_target_properties(pdm PROPERTIES EXPORT_NAME pdm)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pdm)
install(EXPORT pdm-targets
  FILE
    pdmTargets.cmake
  NAMESPACE
    ccp::
  DESTINATION
    ${INSTALL_CONFIGDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/pdmConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/pdmConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/pdmConfig.cmake
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/pdmConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/pdmConfigVersion.cmake
  DESTINATION
    ${INSTALL_CONFIGDIR}
)

export(EXPORT pdm-targets FILE ${CMAKE_CURRENT_BINARY_DIR}/pdmTargets.cmake NAMESPACE ccp::)
export(PACKAGE pdm)

# Testing application

add_executable(pdmCLI
	${SRC_FILES}
	src/main.cpp
)
set_property(TARGET pdmCLI PROPERTY CXX_STANDARD 17)
set_property(TARGET pdmCLI PROPERTY CXX_STANDARD_REQUIRED ON)