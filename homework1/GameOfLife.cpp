#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>
#include <shared_mutex>

#include "../util/CellMatrix.hpp"
#include "../util/ThreadPool.hpp"
#include "../util/ThreadPool2.hpp"

using namespace std;
using namespace util;

#define DEBUG_LOGGING
// #define CELL_MATRIX_DEBUG_LOGGING
// #define CELL_UPDATE_DEBUG_LOGGING


//TODO: integrate a method into the CellMatrix that uses a single mutex to lock all writes to a certain offset of the matrix
//  This will allow the update method to read the entire offset without acquiring any new locks even across multiple threads




bool getCellUpdate(CellMatrix &grid, int row, int column) {
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

void updateCells(CellMatrix &matrix) {
    const int nextOffset = matrix.getNextOffset();
    for (int i = 0; i < matrix.rows(); i++) {
        for (int j = 0; j < matrix.columns(); j++) {
            matrix.set(i, j, getCellUpdate(matrix, i, j), nextOffset);
        }
    }
    matrix.incrementOffset();
}

// TODO: update this to do a block of updates per thread instead of a row and make it smart by using data read in previous updates
void updateCellsUsingThreadPool(CellMatrix &matrix, ThreadPool &threadPool, int numGroups = -1) {
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

    vector<pair<int, int>> rowGroups(numGroups);

    for (int i = 0; i < numGroups; i++) {
        rowGroups.at(i) = make_pair(i*groupSize + previousOverhang, (i+1)*groupSize + allocatedOverhang);

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

    shared_mutex m;

    for (auto & rowGroup : rowGroups) {
    // for (int row = 0; row < matrix.rows(); row++) {
        threadPool.enqueue([nextOffset
            // , row
            , &rowGroup
            , &matrix, &m, &rowGroups] {
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

int main(int argc, char** argv) {
    cout << "Hello World!" << endl;

    constexpr int size = 10000;

    constexpr int iterations = 1000;

    constexpr int printCount = max(iterations / 10, 1);

    constexpr int printThreshold = 101;

    constexpr int numThreads = 8;

    chrono::time_point<chrono::system_clock> start, end;


    CellMatrix matrix = CellMatrix(size);

    #ifdef DEBUG_LOGGING
    cout << "constructor complete" << endl;
    #endif

    matrix.fillWithRandom();

    #ifdef DEBUG_LOGGING
    cout << "fill with random complete" << endl;
    #endif
    // vector<bool> initializer = {
    //     true, true, false,
    //     true, true, false,
    //     false, false, false
    // };

    // matrix.setFromVector(initializer);

    const int sum = matrix.getSum();


    #ifdef DEBUG_LOGGING
    cout << "sum complete" << endl;
    #endif

    if constexpr (size < printThreshold) {
        cout << matrix << endl;
    }

    ThreadPool threadPool(numThreads);

    start = chrono::system_clock::now();

    for (int i = 0; i < iterations; i++) {

        // updateCells(matrix);

        updateCellsUsingThreadPool(matrix, threadPool, numThreads);


        if (i % printCount == 0) {
            cout << "On iteration: " << i << " , " << (i/static_cast<double>(iterations))*100.0  << "%" << endl;
        }
    }

    end = chrono::system_clock::now();

    cout << "System took: " << chrono::duration_cast<chrono::seconds>(end - start).count() << " seconds to run" << endl;

    if constexpr (size < printThreshold) {
        cout << "end matrix " << endl;
        cout << matrix << endl;
    }

    const double percent = (sum / static_cast<double>(size*size)) * 100.0;
    cout << "percent: " << percent << endl;

    return 0;
}