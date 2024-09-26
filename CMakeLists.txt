cmake_minimum_required(VERSION 3.16)
project(CS481_HPC)

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)


add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/GameOfLife")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/util")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/classCode")
