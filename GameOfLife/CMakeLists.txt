
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
    Util
)