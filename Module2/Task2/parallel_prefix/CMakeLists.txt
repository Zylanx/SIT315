cmake_minimum_required(VERSION 3.23)
project(parallel_prefix LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

option(USE_OPENMP "Compile with OpenMP parallelism enabled" ON)

if(USE_OPENMP)
    find_package(OpenMP REQUIRED)
endif()

add_executable(parallel_prefix QuickSort.cpp)

if (OpenMP_CXX_FOUND)
    target_link_libraries(parallel_prefix PRIVATE OpenMP::OpenMP_CXX)
endif()
