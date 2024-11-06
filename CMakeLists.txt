cmake_minimum_required(VERSION 3.16)
project(CS481_HPC)

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)

set(CMAKE_CXX_COMPILER mpicxx)


add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/GameOfLife")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/util")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/classCode")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/MPITesting")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

