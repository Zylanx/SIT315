cmake_minimum_required(VERSION 3.22)
project(Task1)

add_subdirectory("${PROJECT_SOURCE_DIR}/mpi_only" "${PROJECT_SOURCE_DIR}/mpi_only/mpi_only_build")
add_subdirectory("${PROJECT_SOURCE_DIR}/mpi_and_openmp" "${PROJECT_SOURCE_DIR}/mpi_and_openmp/mpi_and_openmp_build")
add_subdirectory("${PROJECT_SOURCE_DIR}/mpi_and_opencl" "${PROJECT_SOURCE_DIR}/mpi_and_opencl/mpi_and_opencl_build")
