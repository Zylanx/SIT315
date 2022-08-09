cmake_minimum_required(VERSION 3.23)
project(multithreaded LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

add_executable(multithreaded VectorAdd.cpp)