add_library(Util
    ThreadPool.cpp
#    CellMatrix.cpp
    FixedSizeQueue.cpp
    ICellMatrix.cpp
    MultiArrayCellMatrix.cpp
    LibraryCode.cpp
        FileIO.hpp
)

target_link_libraries(Util
    PRIVATE Threads::Threads
)