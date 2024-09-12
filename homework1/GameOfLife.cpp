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
// #define CELL_UPDATE_DEBUG_LOGGING



int main(int argc, char** argv) {

    int rows = 5000;
    int columns = rows;

    int iterations = 5000;

    constexpr int maxOffset = 1;

    constexpr int printThreshold = 50;

    constexpr int numThreads = 10;

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

    // matrix.setFromVector(initializer);

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

    auto groups = calculateRowGroups(matrix, numThreads);

    for (int i = 0; i < iterations; i++) {

//        updateCells(matrix);

        bool updateOccurred = updateCellsUsingThreadPool(matrix, threadPool, groups);

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

    const double percent = (sum / static_cast<double>(rows*columns)) * 100.0;
    cout << "percent: " << percent << "%" << endl;

    return 0;
}