cmake_minimum_required(VERSION 3.23)
project(mpi_only LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

option(USE_MPI "Use MPI" ON)

if(USE_MPI)
    find_package(MPI REQUIRED)
endif()

add_executable(${PROJECT_NAME} MatrixMultiply.cpp)

if (MPI_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE MPI::MPI_CXX)
endif()

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_BINDIR})