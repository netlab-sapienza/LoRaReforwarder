cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_CXX_STANDARD 17)

project(loraReforwarder)

include_directories(src)
file(GLOB_RECURSE SRCS src/*.cpp)
add_executable(loraReforwarder ${SRCS})