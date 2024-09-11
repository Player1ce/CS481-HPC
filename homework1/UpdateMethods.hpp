//
// Created by marcelo on 9/11/24.
//

#ifndef CS481_HPC_UPDATEMETHODS_HPP
#define CS481_HPC_UPDATEMETHODS_HPP

#include "../util/CellMatrix.hpp"
#include "../util/ThreadPool.hpp"

#include <mutex>
#include <vector>
#include <string>
#include <iostream>


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


inline bool getCellUpdate(bool currentState, int neighborsAlive) {
#ifdef CELL_UPDATE_DEBUG_LOGGING
    cout << "val: " << neighborsAlive << endl;
#endif

    switch (neighborsAlive) {
        case (-1):
        case (0):
        case (1):
            return false;
            break;

        case(2):
            return currentState;
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
    int neighborsAlive = -grid.get(row, column);

#ifdef CELL_UPDATE_DEBUG_LOGGING
    cout << "[" << endl;
#endif

    for (int i = -1; i <= 1; i++) {
#ifdef CELL_UPDATE_DEBUG_LOGGING
        cout << "[";
#endif

        for (int j = -1; j <=1; j++) {
#ifdef CELL_UPDATE_DEBUG_LOGGING
            cout << " " << grid.get(row + i, column + j);
#endif

            neighborsAlive += grid.get(row + i, column + j);
        }

#ifdef CELL_UPDATE_DEBUG_LOGGING
        cout << "]" << endl;
#endif

        if (neighborsAlive >= 4) {
            break;
        }
    }

#ifdef CELL_UPDATE_DEBUG_LOGGING
    cout << "]" << endl;

    cout << "val: " << neighborsAlive << endl;
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

inline void updateCells(util::CellMatrix &matrix) {
    const int nextOffset = matrix.getNextOffset();
    for (int i = 0; i < matrix.rows(); i++) {
        for (int j = 0; j < matrix.columns(); j++) {
            matrix.set(i, j, getCellUpdate(matrix, i, j), nextOffset);
        }
    }
    matrix.incrementOffset();
}

//TODO: Make a system that reserves acess to a certain set of indices then only apply mutexes to the overlapping ones
//      Specifically, take advantage of the fact that only overlap on actual stored uint64_t values needs protection

// TODO: update this to do a block of updates per thread instead of a row and make it smart by using data read in previous updates
void updateCellsUsingThreadPool(util::CellMatrix &matrix, util::ThreadPool &threadPool, int numGroups = -1) {
    // cout << "Update Started" << endl;

    if (numGroups == -1) {
        numGroups = static_cast<int>(threadPool.getNumThreads());
    } else if (numGroups == -2) {
        numGroups = matrix.rows();
    } else if (numGroups < 0) {
        numGroups = static_cast<int>(threadPool.getNumThreads());
    }

    if (numGroups > matrix.rows()) {
        numGroups = matrix.rows();
    }

    const int nextOffset = matrix.getNextOffset();

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

    // cout << "[";
    // for (int i = 0; i < rowGroups.size(); i++) {
    //     cout << "(" <<rowGroups.at(i).first << ", " << rowGroups.at(i).second << "), ";
    // }
    // cout << "]" << endl;

    std::shared_mutex m;

    for (auto & rowGroup : rowGroups) {
        // for (int row = 0; row < matrix.rows(); row++) {
        threadPool.enqueue([nextOffset
                                   // , row
                                   , &rowGroup
                                   , &matrix, &m] {
            std::vector<int> costTracker(rowGroup.second - rowGroup.first);

            for (int row = rowGroup.first; row < rowGroup.second; row++) {
                for (int col = 0; col < matrix.columns(); col++) {

                    // TODO: need to implement this lock in the CellMatrix at one point for efficiency
                    // shared_lock read_lock(m);
                    const bool update = getCellUpdate(matrix, row, col);
                    // read_lock.unlock();

                    // unique_lock write_lock(m);
                    matrix.set(row, col, update, nextOffset);
                    // write_lock.unlock();

                    // cout << "i: " << i << ", j: " << j << endl;
                }
            }
        });
    }

    // cout << "ThreadPool started" << endl;

    // threadPool.enqueue([] {
    //     this_thread::sleep_for(chrono::seconds(10));
    //     cout << "Thread ID: " << this_thread::get_id() << endl;
    // });

    threadPool.waitTillEmpty();

    // cout << "ThreadPool stopped" << endl;

    matrix.incrementOffset();
}



#endif //CS481_HPC_UPDATEMETHODS_HPP
