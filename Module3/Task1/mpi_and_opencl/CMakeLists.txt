cmake_minimum_required(VERSION 3.22)
project(mpi_and_opencl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

option(USE_MPI "Use MPI" ON)
option(USE_OPENCL "Use OpenCL" ON)

if(USE_MPI)
    find_package(MPI REQUIRED)
endif()

if(USE_OPENCL)
    find_package(OpenCL REQUIRED)
endif()

add_executable(${PROJECT_NAME} MatrixMultiply.cpp MatrixMultiplyCl.h MatrixMultiplyCl.cpp types.h)

if (MPI_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE MPI::MPI_CXX)
endif()

if (OpenCL_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenCL::OpenCL)
endif()

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_BINDIR})
