cmake_minimum_required(VERSION 3.10)

project(Tests)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.10.0
)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_executable(Tests
	Tests.cpp
)

set_property(TARGET Tests PROPERTY CXX_STANDARD 17)
set_property(TARGET Tests PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(Tests LINK_PUBLIC pdm gtest_main)

include(GoogleTest)
enable_testing()
gtest_discover_tests(Tests)