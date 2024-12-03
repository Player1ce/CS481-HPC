set(CMAKE_CUDA_COMPILER /usr/local/cuda-12.6/bin/nvcc)

cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(GameOfLifeCuda LANGUAGES CXX CUDA)

# Find CUDA
find_package(CUDA REQUIRED)

set(CUDA_ARCH_LIST "80;90" CACHE STRING "CUDA architectures to build for")

enable_language(CUDA)

# Find Boost
find_package(Boost REQUIRED COMPONENTS filesystem system)

# Find openMP
find_package(OpenMP REQUIRED)

message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")


set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

# Include directories for Boost
include_directories(${Boost_INCLUDE_DIRS})

add_executable(Homework5 GameOfLifeCuda.cu)

set_target_properties(Homework5 PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
)

#target_compile_options(Homework5 PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-arch=sm_60>)

target_link_libraries(Homework5
        PUBLIC ${Boost_LIBRARIES}
        PRIVATE ${CUDA_LIBRARIES}
        PRIVATE OpenMP::OpenMP_CXX
)


add_executable(CudaTest CudaTest.cu)

set_target_properties(CudaTest PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
)

#target_compile_options(CudaTest PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-arch=sm_60>)

target_link_libraries(CudaTest
        PRIVATE ${CUDA_LIBRARIES}
)


add_executable(Homework3
        GameOfLifeStandalone.cpp
)

target_link_libraries(Homework3
        PUBLIC ${Boost_LIBRARIES}
        PRIVATE OpenMP::OpenMP_CXX
)


