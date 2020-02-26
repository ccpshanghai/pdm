cmake_minimum_required(VERSION 3.8)

project(MetricsLib)

if(NOT APPLE)
	set(CMAKE_INSTALL_RPATH $ORIGIN)
endif()

add_library(MetricsLib SHARED
	Metrics.h
	Metrics.cpp
	Gatherer.h
	Gatherer.cpp
)

set_target_properties(
	MetricsLib PROPERTIES
		SOVERSION 1
		VERSION   1.0.0
)

set_property(TARGET MetricsLib PROPERTY CXX_STANDARD 17)
set_property(TARGET MetricsLib PROPERTY CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

install(TARGETS MetricsLib
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		COMPONENT Metrics_RunTime
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		COMPONENT Metrics_RunTime
		NAMELINK_COMPONENT Metrics_Development
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		COMPONENT Metrics_Development
)

target_include_directories(MetricsLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include(GenerateExportHeader)
generate_export_header(MetricsLib)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
	MetricsLibConfigVersion.cmake
	VERSION 1.0.0
	COMPATIBILITY SameMajorVersion
)