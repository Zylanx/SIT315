cmake_minimum_required(VERSION 3.22)
project(vector_ops LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

option(USE_OPENCL "Compile with Message Passing Interaface support enabled" ON)

if(USE_OPENCL)
    find_package(OpenCL REQUIRED)
endif()

add_executable(${PROJECT_NAME} vector_ops.cpp)

if (OpenCL_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenCL::OpenCL)
endif()


#install(TARGETS ${PROJECT_NAME}
#        DESTINATION ${CMAKE_INSTALL_BINDIR})
