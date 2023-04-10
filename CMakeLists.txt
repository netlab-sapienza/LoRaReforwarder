cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L/usr/local/lib/mariadb -I/usr/local/include")
set(JSON_BuildTests OFF CACHE INTERNAL "")

project(loraReforwarder)

add_subdirectory(third_party/mariadbpp)
add_subdirectory(third_party/json)

include_directories(src)

file(GLOB_RECURSE SRCS src/*.cpp)

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json mariadbcpp)
