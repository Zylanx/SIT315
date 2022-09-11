cmake_minimum_required(VERSION 3.23)
project(MpiVectorAdd LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

option(USE_MPI "Compile with Message Passing Interaface support enabled" ON)

if(USE_MPI)
    find_package(MPI REQUIRED)
endif()

add_executable(${PROJECT_NAME} VectorAdd.cpp)

if (MPI_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE MPI::MPI_CXX)
endif()


install(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_BINDIR})
