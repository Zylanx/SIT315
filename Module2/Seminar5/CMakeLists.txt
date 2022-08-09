cmake_minimum_required(VERSION 3.23)
project(Seminar5)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory("${PROJECT_SOURCE_DIR}/original" "${PROJECT_SOURCE_DIR}/original/original_build")
add_subdirectory("${PROJECT_SOURCE_DIR}/multithreaded" "${PROJECT_SOURCE_DIR}/multithreaded/multithreaded_build")
add_subdirectory("${PROJECT_SOURCE_DIR}/omp_version" "${PROJECT_SOURCE_DIR}/omp_version/omp_build")