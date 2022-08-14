cmake_minimum_required(VERSION 3.23)
project(Task1)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory("${PROJECT_SOURCE_DIR}/sequential" "${PROJECT_SOURCE_DIR}/sequential/sequential_build")
add_subdirectory("${PROJECT_SOURCE_DIR}/std_thread" "${PROJECT_SOURCE_DIR}/std_thread/std_thread_build")
add_subdirectory("${PROJECT_SOURCE_DIR}/omp_version" "${PROJECT_SOURCE_DIR}/omp_version/omp_version_build")
add_subdirectory("${PROJECT_SOURCE_DIR}/combined" "${PROJECT_SOURCE_DIR}/combined/combined_build")