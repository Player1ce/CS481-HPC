cmake_minimum_required(VERSION 3.16)

#set(CMAKE_CXX_COMPILER icpx)

project(CS481_HPC)

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

find_package(OpenMP REQUIRED)

add_library(Util
        CellMatrix.cpp
)

target_link_libraries(Util
        PUBLIC OpenMP::OpenMP_CXX
)

add_executable(Homework2
        GameOfLife.cpp
)

target_link_libraries(Homework2
        Util
)
