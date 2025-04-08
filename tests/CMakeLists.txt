cmake_minimum_required(VERSION 3.31)

project(Tests)

find_package(GTest CONFIG REQUIRED)
add_executable(Tests
	Tests.cpp
)

set_property(TARGET Tests PROPERTY CXX_STANDARD 17)
set_property(TARGET Tests PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(Tests LINK_PUBLIC pdm GTest::gtest_main)