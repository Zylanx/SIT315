cmake_minimum_required(VERSION 3.22)
project(mpi_and_openmp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

option(USE_MPI "Use MPI" ON)
option(USE_OPENMP "Use OpenMP" ON)

if(USE_MPI)
    find_package(MPI REQUIRED)
endif()

if(USE_OPENMP)
    find_package(OpenMP REQUIRED)
endif()

add_executable(${PROJECT_NAME} MatrixMultiply.cpp)

if (MPI_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE MPI::MPI_CXX)
endif()

if (OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
endif()

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_BINDIR})
