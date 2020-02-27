cmake_minimum_required(VERSION 3.8)

project(Metrics_CLI)

add_executable(Metrics_CLI
	main.cpp
)

target_link_libraries(Metrics_CLI LINK_PUBLIC MetricsLib)