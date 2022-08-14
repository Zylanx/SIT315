cmake_minimum_required(VERSION 3.23)
project(combined LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

option(USE_OPENMP "Compile with OpenMP parallelism enabled" ON)

if(USE_OPENMP)
    find_package(OpenMP REQUIRED)
endif()

add_executable(combined MatrixMultiply.cpp)

if (OpenMP_CXX_FOUND)
    target_link_libraries(combined PRIVATE OpenMP::OpenMP_CXX)
endif()
