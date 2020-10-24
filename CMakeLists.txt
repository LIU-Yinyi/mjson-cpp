cmake_minimum_required(VERSION 3.15)
project(mjson_cpp)

####################################
# options for ccmake
option(MJSON_ADDON_EIGEN "mjson-cpp add-ons with Eigen3 library" OFF)
option(MJSON_BUILD_DOC "mjson-cpp build doxygen document" OFF)
option(MJSON_BUILD_RELEASE "mjson-cpp build in release(ON) or debug(OFF)" OFF)
option(MJSON_BUILD_TEST "mjson-cpp build test applications" OFF)

####################################
# find packages

####################################
# configuration for common cases
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-missing-field-initializers")

set(MJSON_INCLUDES ${CMAKE_CURRENT_BINARY_DIR} include include/mjson include/rapidjson)

####################################
# configuration based on options
if (MJSON_ADDON_EIGEN)
    find_package(Eigen3 REQUIRED)
    set(MJSON_INCLUDES ${MJSON_INCLUDES} ${EIGEN3_INCLUDE_DIRS})
    message("[INFO] Use Add-Ons of Eigen3.")
endif ()

if (MJSON_BUILD_RELEASE)
    set(CMAKE_BUILD_TYPE Release)
    message("[INFO] Set Build Type as RELEASE.")
else()
    set(CMAKE_BUILD_TYPE Debug)
    message("[INFO] Set Build Type as DEBUG.")
endif()

####################################
# configuration for compilation
configure_file(config.h.in config.h @ONLY)

include_directories(${MJSON_INCLUDES})
add_executable(mjson_cpp example.cpp)