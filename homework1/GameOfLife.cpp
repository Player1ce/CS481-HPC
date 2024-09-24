/*
Name: Marcelo Torres
Email: mtorres3@crimson.ua.edu
Course Section: CS 481
Homework #: 1
Instructions to compile the program: (for example: gcc -Wall -O -o hw1 hw1.c)
Instructions to run the program: (for example: ./hw1 <size> <iterations>)
*/

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>
#include <shared_mutex>

#include "../util/CellMatrix.hpp"
#include "../util/ThreadPool.hpp"

#include "UpdateMethods.hpp"

using namespace std;
using namespace util;

#define DEBUG_LOGGING



int main(int argc, char** argv) {

    int rows = 1000;
    int columns = rows;

    int iterations = 5000;

    constexpr bool useInitializerList = false;

    constexpr int maxOffset = 1;

    constexpr int printThreshold = 50;

    int numThreads = 10;

    if (argc < 3) {
        cout << "Using coded constants" << endl;
    }
    else if (argc == 3) {
        cout << "Using size: " << argv[1] << " and iterations: " << argv[2] << endl;
        rows = atoi(argv[1]);
        columns = rows;
        iterations = atoi(argv[2]);
    }
    else if (argc == 4) {
        cout << "Using rows: " << argv[1] << " and columns: " << argv[2] << " and iterations: " << argv[3] << endl;
        rows = atoi(argv[1]);
        columns = atoi(argv[2]);
        iterations = atoi(argv[3]);
    }
    else if (argc == 5) {
        cout << "Using rows: " << argv[1] << " and columns: " << argv[2] << " and iterations: " << argv[3] << " and numThreads: " << argv[4] << endl;
        rows = atoi(argv[1]);
        columns = atoi(argv[2]);
        iterations = atoi(argv[3]);
        numThreads = atoi(argv[4]);
    }

    int printCount = max(iterations / 10, 1);

    chrono::time_point<chrono::system_clock> start, end;

    CellMatrix matrix = CellMatrix(rows, columns, maxOffset);

    #ifdef DEBUG_LOGGING
    cout << "constructor complete" << endl;
    #endif

    matrix.fillWithRandom();

    #ifdef DEBUG_LOGGING
    cout << "fill with random complete" << endl;
    #endif

    vector<bool> initializer = {
            true, true, false,
            true, true, false,
            false, false, false
    };

    vector<bool> initializer2 = {
            true, true, false, false, true,
            false, true, true, false, false,
            true, true, false, false, true,
            false, true, true, true, true,
            true, true, true, false, true
    };

    vector<bool> test2 = {
        true, true, true, false, false,
        false, false, true, true, false,
        true, false, false, false, true,
        false, false, false, false, true,
        true, false, false, false, true,
    };

    if (useInitializerList) {
//        matrix.fillFromVector(initializer);
        matrix.fillFromVector(initializer2);
    }

    const int sum = matrix.getSum();


    #ifdef DEBUG_LOGGING
    cout << "sum complete" << endl;
    #endif

    if (rows*columns < printThreshold * printThreshold) {
        cout << matrix << endl;
    }

     ThreadPool threadPool(numThreads);

    int multiplier = 1;

    start = chrono::system_clock::now();

    bool updateOccurred;

    auto groups = calculateRowGroups(matrix, numThreads);

    for (int i = 0; i < iterations; i++) {

//        updateOccurred = updateCells(matrix);

//        updateOccurred = updateCellsUsingThreadPool(matrix, threadPool, groups);

        updateOccurred = updateCellsUsingThreadPoolOptimized(matrix, threadPool, groups);

        if (i == printCount * multiplier) {
            cout << "On iteration: " << i << " , " << (i/static_cast<double>(iterations))*100.0  << "%" << endl;
              multiplier++;
        }

        if (!updateOccurred) {
            cout << "exiting early because no update" << endl;
            break;
        }
    }

    end = chrono::system_clock::now();

    long seconds = chrono::duration_cast<chrono::seconds>(end - start).count();
    long ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    long decimal = ms - seconds * 1000;
    cout << "System took: " << seconds << "." << decimal << " seconds to run" << endl;

    if (rows*columns < printThreshold*printThreshold) {
        cout << "end matrix " << endl;
        cout << matrix << endl;
    }

    if (useInitializerList) {
        if (matrix.rows() * matrix.columns() < printThreshold * printThreshold) {
            bool success = true;

            for (int i = 0; i < matrix.rows(); i++) {
                for (int j = 0; j < matrix.columns(); j++) {
                    if (test2.at((i * rows) + j) != matrix.get(i, j)) {
                        success = false;
                        break;
                    }
                }
            }

            cout << "Success2: " << boolalpha << success << endl;
        }
    }

    const double percent = (sum / static_cast<double>(rows*columns)) * 100.0;
    cout << "percent: " << percent << "%" << endl;

    return 0;
}