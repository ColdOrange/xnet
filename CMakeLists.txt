cmake_minimum_required(VERSION 3.2)
project(xnet)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wno-unused-parameter -O2")

include_directories(${PROJECT_SOURCE_DIR})

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")

add_subdirectory(xnet)
