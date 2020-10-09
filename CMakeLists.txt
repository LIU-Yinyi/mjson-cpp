cmake_minimum_required(VERSION 3.15)
project(mjson_cpp)

find_package(rapidjson REQUIRED)
find_package(Eigen3 REQUIRED)

set(CMAKE_CXX_STANDARD 17)

include_directories(header-only ${RAPIDJSON_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIRS})
add_executable(mjson_cpp main.cpp)