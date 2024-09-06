add_library(Util
    ThreadPool.cpp
    CellMatrix.cpp
        ThreadPool2.cpp
        ThreadPool2.hpp
)

target_link_libraries(Util
        PRIVATE Threads::Threads
)