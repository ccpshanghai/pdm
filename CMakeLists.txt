cmake_minimum_required(VERSION 3.7)

project(pdm VERSION 1.0.2)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

# Pass version info to code
configure_file(include/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h)

set(SRC_FILES 
	src/pdm.cpp
	src/gatherer.cpp
	src/wine/wine.cpp
	src/windows/windows_data.cpp
	src/windows/d3d11_info.cpp
	src/windows/vulkan_info.cpp
	src/macos/macos_data.mm
)

# For IDE support
set(HDR_FILES 
  include/pdm.h
  include/pdm_data.h
  src/defines.h
  src/gatherer.h
  src/windows/d3d11_info.h
  src/windows/vulkan_defs.h
)

add_library(pdm STATIC
	${SRC_FILES}
	${HDR_FILES}
)
add_library(ccp::pdm ALIAS pdm)

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


add_executable(pdmCLI
	${SRC_FILES}
	${HDR_FILES}
	src/main.cpp
)

set_property(TARGET pdmCLI PROPERTY CXX_STANDARD 17)
set_property(TARGET pdmCLI PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET pdm    PROPERTY CXX_STANDARD 17)
set_property(TARGET pdm    PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET pdm    PROPERTY CXX_VISIBILITY_PRESET hidden)
set_property(TARGET pdm    PROPERTY VISIBILITY_INLINES_HIDDEN YES)

if (APPLE)
  find_library(IOKit IOKit)
  find_library(AppKit AppKit)
  find_library(Metal Metal)
  target_link_libraries(pdmCLI ${IOKit} ${AppKit} ${Metal})
  target_link_libraries(pdm    ${IOKit} ${AppKit} ${Metal})
endif()
