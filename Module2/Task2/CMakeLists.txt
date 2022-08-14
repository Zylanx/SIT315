cmake_minimum_required(VERSION 3.23)
project(Task2)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory("${PROJECT_SOURCE_DIR}/sequential" "${PROJECT_SOURCE_DIR}/sequential/sequential_build")
add_subdirectory("${PROJECT_SOURCE_DIR}/omp_version" "${PROJECT_SOURCE_DIR}/omp_version/omp_version_build")