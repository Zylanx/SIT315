cmake_minimum_required(VERSION 3.22)
project(Task3)

add_subdirectory("${PROJECT_SOURCE_DIR}/sequential" "${PROJECT_SOURCE_DIR}/sequential/sequential_build")
add_subdirectory("${PROJECT_SOURCE_DIR}/multithreaded" "${PROJECT_SOURCE_DIR}/multithreaded/multithreaded_build")
