cmake_minimum_required(VERSION 3.22)
project(multithreaded LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME} TrafficControl.cpp TrafficConsumer.h TrafficProducer.h types.h TrafficFile.h TrafficData.h config.h TrafficStore.h)

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_BINDIR})
