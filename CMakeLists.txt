cmake_minimum_required(VERSION 3.8)

project(MetricsLib)

if (MSVC)
	add_compile_options(-W4)
else()
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

if(NOT APPLE)
	set(CMAKE_INSTALL_RPATH $ORIGIN)
endif()

set(CPP_FILES 
	Metrics.cpp
	Gatherer.cpp
	Wine/Wine.cpp
	Windows/WindowsMetrics.cpp
	Windows/D3D11Info.cpp
	Windows/VulkanInfo.cpp
)

add_library(MetricsLib SHARED
	${CPP_FILES}
)

set_target_properties(
	MetricsLib PROPERTIES
		SOVERSION 1
		VERSION   1.0.0
)

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

add_executable(MetricsCLI
	${CPP_FILES}
	main.cpp
)