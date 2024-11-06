
add_executable(GameOfLife
        GameOfLife.cpp
)

target_link_libraries(GameOfLife
    Util
)

add_executable(GameOfLifeStandalone
        GameOfLifeStandalone.cpp
)

target_link_libraries(GameOfLifeStandalone
    PRIVATE Util
    PRIVATE OpenMP::OpenMP_CXX
)

add_executable(GameOfLifeMPI
        GameOfLifeMPI.cpp
)

target_link_libraries(GameOfLifeMPI
        PRIVATE Util
        PRIVATE MPI::MPI_CXX
        PRIVATE OpenMP::OpenMP_CXX
)
