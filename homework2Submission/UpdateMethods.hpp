//
// Created by marcelo on 9/11/24.
//

#ifndef CS481_HPC_UPDATEMETHODS_HPP
#define CS481_HPC_UPDATEMETHODS_HPP

#include "CellMatrix.hpp"

#include <mutex>
#include <vector>
#include <string>
#include <atomic>
#include <iostream>


//TODO: integrate a method into the CellMatrix that uses a single mutex to lock all writes to a certain offset of the matrix
//  This will allow the update method to read the entire offset without acquiring any new locks even across multiple threads

//TODO: solution to mutex problem. Treat all bits on the edge of a range as shared and use atomic_ref to access and edit them.
//      Should be faster than a mutex and equivalently safe. Do calculations to figure out what range is in danger


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
    #ifdef CELL_UPDATE_DEBUG_LOGGING
    cout << "[" << endl;
    #endif

    int neighborsAlive = grid.get(row - 1, column - 1) + grid.get(row - 1, column) + grid.get(row - 1, column + 1) +
                      grid.get(row    , column - 1) +                           + grid.get(row    , column + 1);

    if (neighborsAlive >= 4) return false;

    neighborsAlive += grid.get(row + 1, column - 1) + grid.get(row + 1, column) + grid.get(row + 1, column + 1);

//    for (int i = -1; i <= 1; i++) {
//    #ifdef CELL_UPDATE_DEBUG_LOGGING
//        cout << "[";
//    #endif
//
//        for (int j = -1; j <=1; j++) {
//    #ifdef CELL_UPDATE_DEBUG_LOGGING
//            cout << " " << grid.get(row + i, column + j);
//    #endif
//            if (i != 0 || j != 0) {
//                neighborsAlive += grid.get(row + i, column + j);
//            }
//        }
//
//    #ifdef CELL_UPDATE_DEBUG_LOGGING
//        cout << "]" << endl;
//    #endif
//
//        if (neighborsAlive >= 4) {
//            return false;
//        }
//    }

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


bool updateCells_updateTracked(util::CellMatrix &matrix) {
    // cout << "Update Started" << endl;

    std::atomic<bool> updateOccured = false;

    const int nextOffset = matrix.getNextOffset();

    // cout << "[";
    // for (int i = 0; i < rowGroups.size(); i++) {
    //     cout << "(" <<rowGroups.at(i).first << ", " << rowGroups.at(i).second << "), ";
    // }
    // cout << "]" << endl;
    bool updateStored = false;

    for (int row = 0; row < matrix.rows(); row++) {
        for (int col = 0; col < matrix.columns(); col++) {
            const bool update = getCellUpdate(matrix, row, col);
            if (matrix.set(row, col, update, nextOffset) && !updateStored) {
                updateOccured = true;
                updateStored = true;
            }
            // cout << "i: " << i << ", j: " << j << endl;
        }
    }

    // cout << "ThreadPool started" << endl;

    // threadPool.enqueue([] {
    //     this_thread::sleep_for(chrono::seconds(10));
    //     cout << "Thread ID: " << this_thread::get_id() << endl;
    // });

    // cout << "ThreadPool stopped" << endl;

    matrix.incrementOffset();
    return updateOccured;
}



#endif //CS481_HPC_UPDATEMETHODS_HPP
