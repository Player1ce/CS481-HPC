cmake_minimum_required(VERSION 3.16)
project(CS481_HPC)

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

#find_package(Threads REQUIRED)

add_library(Util
#        ThreadPool.cpp
        CellMatrix.cpp
)

#target_link_libraries(Util
#        PRIVATE Threads::Threads
#)

add_executable(Homework2
        GameOfLife.cpp
)

target_link_libraries(Homework2
        Util
)
