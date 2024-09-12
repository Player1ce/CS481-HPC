add_library(Util
    ThreadPool.cpp
    CellMatrix.cpp
    FixedSizeQueue.cpp
)

target_link_libraries(Util
        PRIVATE Threads::Threads
)