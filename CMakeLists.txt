CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

project(safecc)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")

string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)
if (uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    message("[Debugging mode enabled]")
    add_definitions(-DMODE_DEBUG)
endif ()

include_directories(compiler/include)
file(GLOB_RECURSE SOURCES "compiler/src/*.cpp")

add_executable(safecc ${SOURCES})