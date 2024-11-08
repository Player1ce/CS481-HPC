cmake_minimum_required(VERSION 3.16)

project(CS481_HPC)

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)

set(CMAKE_CXX_COMPILER mpicxx)

add_executable(Homework3
        GameOfLifeStandalone.cpp
)

target_link_libraries(Homework3
    PRIVATE OpenMP::OpenMP_CXX
)

add_executable(Homework4
        GameOfLifeMPI.cpp
)

target_link_libraries(Homework4
        PRIVATE MPI::MPI_CXX
        PRIVATE OpenMP::OpenMP_CXX
)
