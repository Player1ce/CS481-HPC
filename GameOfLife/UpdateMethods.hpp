//
// Created by marcelo on 9/11/24.
//

#ifndef CS481_HPC_UPDATEMETHODS_HPP
#define CS481_HPC_UPDATEMETHODS_HPP

#include "../util/CellMatrix.hpp"
#include "../util/ThreadPool.hpp"
#include "../util/FixedSizeQueue.hpp"

#include <mutex>
#include <vector>
#include <string>
#include <atomic>
#include <iostream>

//#define DEBUG_LOGGING
//#define CELL_UPDATE_DEBUG_LOGGING


//TODO: integrate a method into the CellMatrix that uses a single mutex to lock all writes to a certain offset of the matrix
//  This will allow the update method to read the entire offset without acquiring any new locks even across multiple threads

//TODO: solution to mutex problem. Treat all bits on the edge of a range as shared and use atomic_ref to access and edit them.
//      Should be faster than a mutex and equivalently safe. Do calculations to figure out what range is in danger
inline bool updateCellsRowOptimized(util::CellMatrix matrix,
                                    int rowStart, int rowEnd, int colStart, int colEnd) {
    std::vector<int> currentCol;
    std::vector<int> nextCol;
    std::vector<int> futureCol;

    currentCol.resize(rowEnd - rowStart);
    nextCol.resize(rowEnd - rowStart);
    futureCol.resize(rowEnd - rowStart);

    // initialize
    for (int col = colStart; col < colEnd; col++) {
        currentCol.at(0) += matrix.get(rowStart - 1, colStart - 1);
    }
    currentCol.at(0) += matrix.get(rowStart - 1, colStart - 1);

    for (int colOffset = 0; colOffset <= 1; colOffset++) {
        currentCol.at(0) += matrix.get(rowStart - 1, colStart + colOffset);
        currentCol.at(1) += matrix.get(rowStart - 1, colStart + colOffset);
    }



    // for (int row = rowStart; row < rowEnd; row++) {
    //     for(int col = colStart; col < colEnd; col++) {
    //         switch()
    //     }
    // }



    for (int j = -1; j <= 1; j++) {
        for (int i = -1; i <= 1; i++) {
            currentCol.at(0) += matrix.get(rowStart - 1, colStart);
            nextCol.at(0) += matrix.get(rowStart - 1, colStart);
        }
    }


    for (int row = rowStart; row < rowEnd; row++) {
        for (int colOffset = -1; colOffset <=1; colOffset++) {
            #ifdef CELL_UPDATE_DEBUG_LOGGING
            // cout << " " << grid.get(row + rowOffset, column + colOffset);
            #endif
            // neighborsAlive += grid.get(rowStart + rowOffset, colStart + colOffset);
        }
    }
    return 0;

}


inline bool getCellUpdate(util::CellMatrix &grid, int row, int column, int neighborsAlive) {
    #ifdef CELL_UPDATE_DEBUG_LOGGING
    std::cout <<" | val: " << neighborsAlive << " ";
    #endif

    switch (neighborsAlive) {
        case (-1):
        case (0):
        case (1):
            return false;
            break;

        case(2):
            return grid.get(row, column) == 1;;
            break;

        case(3):
            return true;
            break;

        default:
            return false;
            break;
    }
}


inline bool getCellUpdateSubtractingState(int neighborsAlive, const bool cellAlive) {

    neighborsAlive -= (cellAlive ? 1 : 0);

    #ifdef CELL_UPDATE_DEBUG_LOGGING
    std::cout <<" | val: " << neighborsAlive << " ";
    #endif

    switch (neighborsAlive) {
        case (-1):
        case (0):
        case (1):
            return false;
            break;

        case(2):
            return cellAlive;
            break;

        case(3):
            return true;
            break;

        default:
            return false;
            break;
    }
}


inline bool getCellUpdate(util::CellMatrix &grid, int row, int column) {
    int neighborsAlive = 0;

    #ifdef CELL_UPDATE_DEBUG_LOGGING
    std::cout << "[" << std::endl;
    #endif

    for (int i = -1; i <= 1; i++) {
    #ifdef CELL_UPDATE_DEBUG_LOGGING
        std::cout << "[";
    #endif

        for (int j = -1; j <=1; j++) {
    #ifdef CELL_UPDATE_DEBUG_LOGGING
            std::cout << " " << grid.get(row + i, column + j);
    #endif
            if (i != 0 || j != 0) {
                neighborsAlive += grid.get(row + i, column + j);
            }
        }

    #ifdef CELL_UPDATE_DEBUG_LOGGING
        std::cout << "]" << std::endl;
    #endif

        if (neighborsAlive >= 4) {
            break;
        }
    }

    #ifdef CELL_UPDATE_DEBUG_LOGGING
    std::cout << "]" << std::endl;

    std::cout << "val: " << neighborsAlive << std::endl;
//    std::cout <<" | val: " << neighborsAlive << " ";
    #endif

    switch (neighborsAlive) {
        case (-1):
        case (0):
        case (1):
            return false;
            break;

        case(2):
            return grid.get(row, column) == 1;
            break;

        case(3):
            return true;
            break;

        default:
            return false;
            break;
    }
}

inline bool updateCells(util::CellMatrix &matrix) {
    bool update = false;

    const int nextOffset = matrix.getNextOffset();
    for (int i = 0; i < matrix.rows(); i++) {
        for (int j = 0; j < matrix.columns(); j++) {
            update |= matrix.set(i, j, getCellUpdate(matrix, i, j), nextOffset);
        }
    }
    matrix.incrementOffset();

//    return update;
    return true; // TODO: fix this in set
}

//TODO: Make a system that reserves acess to a certain set of indices then only apply mutexes to the overlapping ones
//      Specifically, take advantage of the fact that only overlap on actual stored uint64_t values needs protection


bool updateCells_Windows(util::CellMatrix &matrix) {
    // std::cout << "Update Started" << std::endl;

    bool updateOccurred = false;

    const int nextOffset = matrix.getNextOffset();
    const int currentOffset = matrix.getOffset();

    // TODO: need to expand cellGroups by 1 on all sides with new neighbor placement method

    // std::cout << "[";
    // for (int i = 0; i < rowGroups.size(); i++) {
    //     std::cout << "(" <<rowGroups.at(i).first << ", " << rowGroups.at(i).second << "), ";
    // }
    // std::cout << "]" << std::endl;

    util::FixedSizeQueue<int, 3> windowTracker(true);

//            auto evaluator = [&windowTracker](const bool cellAlive) {
//                return getCellUpdateSubtractingState(windowTracker.sum(), cellAlive);
//            };

    for (int row = 0; row < matrix.rows(); row++) {

        windowTracker.resetQueue();

        windowTracker.push(0);
        windowTracker.push(matrix.getVerticalWindow(row, 0));

        for (int col = 1; col < matrix.columns(); col++) {

            windowTracker.push(matrix.getVerticalWindow(row, col));

//                    std::cout << "sum: " << windowTracker.sum() << " | ";
//                    std::cout << std::endl;

//                    const bool workingUpdate = getCellUpdate(matrix, row, col);
//                    std::cout << "workingUpdate: " << workingUpdate << " | ";

            const bool updateMethod1 = getCellUpdateSubtractingState(windowTracker.sum(), matrix.get(row, col-1));


//                    #ifdef CELL_UPDATE_DEBUG_LOGGING
//                    std::cout << "updateMethod1: " << updateMethod1 << " | ";
//                    #endif

            if (matrix.set(row, col-1, updateMethod1, nextOffset) && !updateOccurred) {
                updateOccurred = true;
            }

        }

        windowTracker.push(0);

//                std::cout << "sum: " << windowTracker.sum() << std::endl;

//                    const bool workingUpdate = getCellUpdate(matrix, row, col);
//                    std::cout << "workingUpdate: " << workingUpdate << " | ";

        const bool updateMethod1 = getCellUpdateSubtractingState(windowTracker.sum(), matrix.get(row, matrix.columns() - 1));


//                    #ifdef CELL_UPDATE_DEBUG_LOGGING
//                    std::cout << "updateMethod1: " << updateMethod1 << " | ";
//                    #endif

        if (matrix.set(row, matrix.columns()-1, updateMethod1, nextOffset) && !updateOccurred) {
            updateOccurred = true;
        }

    }

    // std::cout << "ThreadPool started" << std::endl;

    // threadPool.enqueue([] {
    //     this_thread::sleep_for(chrono::seconds(10));
    //     std::cout << "Thread ID: " << this_thread::get_id() << std::endl;
    // });

    // std::cout << "ThreadPool stopped" << std::endl;

    matrix.incrementOffset();

    return updateOccurred;
}

bool updateCellsUsingThreadPool_Windows(util::CellMatrix &matrix, util::ThreadPool &threadPool, std::vector<std::pair<int, int>>& rowGroups) {
    // std::cout << "Update Started" << std::endl;

    std::atomic<bool> updateOccurred = false;

    const int nextOffset = matrix.getNextOffset();
    const int currentOffset = matrix.getOffset();

    // TODO: need to expand cellGroups by 1 on all sides with new neighbor placement method

    // std::cout << "[";
    // for (int i = 0; i < rowGroups.size(); i++) {
    //     std::cout << "(" <<rowGroups.at(i).first << ", " << rowGroups.at(i).second << "), ";
    // }
    // std::cout << "]" << std::endl;


    for (auto & rowGroup : rowGroups) {
        threadPool.enqueue([nextOffset, currentOffset
                                   , &rowGroup
                                   , &matrix
                                   , &updateOccurred] {
            bool updateStored = false;
            util::FixedSizeQueue<int, 3> windowTracker(true);

//            auto evaluator = [&windowTracker](const bool cellAlive) {
//                return getCellUpdateSubtractingState(windowTracker.sum(), cellAlive);
//            };

            for (int row = rowGroup.first; row < rowGroup.second; row++) {
                windowTracker.resetQueue();

                windowTracker.push(0);
                windowTracker.push(matrix.getVerticalWindow(row, 0));

                for (int col = 1; col < matrix.columns(); col++) {

                    windowTracker.push(matrix.getVerticalWindow(row, col));

//                    std::cout << "sum: " << windowTracker.sum() << " | ";
//                    std::cout << std::endl;

//                    const bool workingUpdate = getCellUpdate(matrix, row, col);
//                    std::cout << "workingUpdate: " << workingUpdate << " | ";

                    const bool updateMethod1 = getCellUpdateSubtractingState(windowTracker.sum(), matrix.get(row, col-1));


//                    #ifdef CELL_UPDATE_DEBUG_LOGGING
//                    std::cout << "updateMethod1: " << updateMethod1 << " | ";
//                    #endif

                    if (matrix.set(row, col-1, updateMethod1, nextOffset) && !updateStored) {
                        updateOccurred = true;
                        updateStored = true;
                    }

                }

                windowTracker.push(0);

//                std::cout << "sum: " << windowTracker.sum() << std::endl;
    
//                    const bool workingUpdate = getCellUpdate(matrix, row, col);
//                    std::cout << "workingUpdate: " << workingUpdate << " | ";

                const bool updateMethod1 = getCellUpdateSubtractingState(windowTracker.sum(), matrix.get(row, matrix.columns() - 1));


//                    #ifdef CELL_UPDATE_DEBUG_LOGGING
//                    std::cout << "updateMethod1: " << updateMethod1 << " | ";
//                    #endif

                if (matrix.set(row, matrix.columns()-1, updateMethod1, nextOffset) && !updateStored) {
                    updateOccurred = true;
                    updateStored = true;
                }

            }
        });
    }

    // std::cout << "ThreadPool started" << std::endl;

    // threadPool.enqueue([] {
    //     this_thread::sleep_for(chrono::seconds(10));
    //     std::cout << "Thread ID: " << this_thread::get_id() << std::endl;
    // });

    threadPool.waitTillEmpty();

    // std::cout << "ThreadPool stopped" << std::endl;

    matrix.incrementOffset();

    return updateOccurred;
}

std::vector<std::pair<int, int>> calculateRowGroups(util::CellMatrix &matrix, int numGroups) {

    if (numGroups < 0) {
        numGroups = 1;
    }

    if (numGroups > matrix.rows()) {
        numGroups = matrix.rows();
    }

    int groupSize = matrix.rows()/numGroups;
    int overhang = matrix.rows()%numGroups;
    int previousOverhang = 0;
    int allocatedOverhang = 0;

    std::vector<std::pair<int, int>> rowGroups(numGroups);

    for (int i = 0; i < numGroups; i++) {
        rowGroups.at(i) = std::make_pair(i*groupSize + previousOverhang, (i+1)*groupSize + allocatedOverhang);

        if (i < overhang) {
            rowGroups.at(i).second += 1;
            previousOverhang = 1;
            allocatedOverhang++;
        } else {
            previousOverhang = 0;
        }
    }

    return rowGroups;
}


bool updateCellsUsingThreadPool(util::CellMatrix &matrix, util::ThreadPool &threadPool, std::vector<std::pair<int, int>>& rowGroups) {
    // std::cout << "Update Started" << std::endl;

    // std::cout << "[";
    // for (int i = 0; i < rowGroups.size(); i++) {
    //     std::cout << "(" <<rowGroups.at(i).first << ", " << rowGroups.at(i).second << "), ";
    // }
    // std::cout << "]" << std::endl;

    std::atomic<bool> updateOccurred = false;

    const int nextOffset = matrix.getNextOffset();

    std::shared_mutex m;

    for (auto & rowGroup : rowGroups) {
        // for (int row = 0; row < matrix.rows(); row++) {
        threadPool.enqueue([nextOffset
                                   // , row
                                   , &rowGroup
                                   , &matrix, &m
                                   , &updateOccurred] {
//            std::vector<int> costTracker(rowGroup.second - rowGroup.first);
            bool updateStored = false;

            for (int row = rowGroup.first; row < rowGroup.second; row++) {
                for (int col = 0; col < matrix.columns(); col++) {

                    // TODO: need to implement this lock in the CellMatrix at one point for efficiency

                    const bool update = getCellUpdate(matrix, row, col);

                    if (matrix.set(row, col, update, nextOffset) && !updateStored) {
                        updateOccurred = true;
                        updateStored = true;
                    }

                }
            }
        });
    }

    // std::cout << "ThreadPool started" << std::endl;

    // threadPool.enqueue([] {
    //     this_thread::sleep_for(chrono::seconds(10));
    //     std::cout << "Thread ID: " << this_thread::get_id() << std::endl;
    // });

    threadPool.waitTillEmpty();

    // std::cout << "ThreadPool stopped" << std::endl;

    matrix.incrementOffset();
    return updateOccurred;
}


bool updateCellsUsingThreadPool_Windows_Optimized(util::CellMatrix &matrix, util::ThreadPool &threadPool, std::vector<std::pair<int, int>>& rowGroups) {
    // std::cout << "Update Started" << std::endl;

    std::atomic<bool> updateOccurred = false;

    const int nextOffset = matrix.getNextOffset();
    const int currentOffset = matrix.getOffset();

    // TODO: need to expand cellgroups by 1 on all sides with new neighbor placement method

    // std::cout << "[";
    // for (int i = 0; i < rowGroups.size(); i++) {
    //     std::cout << "(" <<rowGroups.at(i).first << ", " << rowGroups.at(i).second << "), ";
    // }
    // std::cout << "]" << std::endl;


    for (auto & rowGroup : rowGroups) {
        threadPool.enqueue([nextOffset, currentOffset
                                   , &rowGroup
                                   , &matrix
                                   , &updateOccurred] {
            bool updateStored = false;
            util::FixedSizeQueue<int, 3> windowTracker;

//            auto evaluator = [&windowTracker](const bool cellAlive) {
//                return getCellUpdateSubtractingState(windowTracker.sum(), cellAlive);
//            };

            for (int row = rowGroup.first; row < rowGroup.second; row++) {
                windowTracker.push(0);
                for (int col = 0; col < matrix.columns() + 1; col++) {

                    windowTracker.push(matrix.getVerticalWindow(row, col));

//                    std::cout << "sum: " << windowTracker.sum() << " | ";
//                    std::cout << std::endl;

//                    const bool workingUpdate = getCellUpdate(matrix, row, col);
//                    std::cout << "workingUpdate: " << workingUpdate << " | ";

                    const bool updateMethod1 = getCellUpdateSubtractingState(windowTracker.sum(), matrix.get(row, col-1));


//                    #ifdef CELL_UPDATE_DEBUG_LOGGING
//                    std::cout << "updateMethod1: " << updateMethod1 << " | ";
//                    #endif

                    if (matrix.set(row, col-1, updateMethod1, nextOffset) && !updateStored) {
                        updateOccurred = true;
                        updateStored = true;
                    }

                }
            }
        });
    }

    // std::cout << "ThreadPool started" << std::endl;

    // threadPool.enqueue([] {
    //     this_thread::sleep_for(chrono::seconds(10));
    //     std::cout << "Thread ID: " << this_thread::get_id() << std::endl;
    // });

    threadPool.waitTillEmpty();

    // std::cout << "ThreadPool stopped" << std::endl;

    matrix.incrementOffset();

    return updateOccurred;
}



#endif //CS481_HPC_UPDATEMETHODS_HPP
