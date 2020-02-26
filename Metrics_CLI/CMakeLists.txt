cmake_minimum_required(VERSION 3.8)

project(Metrics_CLI)

add_executable(Metrics_CLI
	Metrics_CLI.cpp
	Metrics_CLI.h
)

set_property(TARGET Metrics_CLI PROPERTY CXX_STANDARD 17)
set_property(TARGET Metrics_CLI PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(Metrics_CLI LINK_PUBLIC MetricsLib)