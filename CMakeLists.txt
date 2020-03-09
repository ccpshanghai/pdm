cmake_minimum_required(VERSION 3.8)

project(libPDM)

set(CPP_FILES 
	PDM.cpp
	Gatherer.cpp
	Wine/Wine.cpp
	Windows/WindowsData.cpp
	Windows/D3D11Info.cpp
	Windows/VulkanInfo.cpp
	MacOS/MacOSData.cpp
)

if (MSVC)
	add_compile_options(-W4)
else()
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_PDB_OUTPUT_DIRECTORY     ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

if(NOT APPLE)
	set(CMAKE_INSTALL_RPATH $ORIGIN)
endif()

add_library(libPDM SHARED
	${CPP_FILES}
)

set_target_properties(
	libPDM PROPERTIES
		SOVERSION 1
		VERSION   1.0.0
)

install(TARGETS libPDM
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		COMPONENT PDM_RunTime
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		COMPONENT PDM_RunTime
		NAMELINK_COMPONENT PDM_Development
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		COMPONENT PDM_Development
)

target_include_directories(libPDM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include(GenerateExportHeader)
generate_export_header(libPDM)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
	PDMLibConfigVersion.cmake
	VERSION 1.0.0
	COMPATIBILITY SameMajorVersion
)

add_executable(PDMCLI
	${CPP_FILES}
	main.cpp
)