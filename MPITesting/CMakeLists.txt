add_executable(MPITest
    MPITest.cpp
)

target_link_libraries(MPITest
        PRIVATE MPI::MPI_CXX
)