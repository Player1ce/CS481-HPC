//
// Created by motst on 10/3/2024.
//

#include "../util/ICellMatrix.hpp"
#include "../util/MultiArrayCellMatrix.hpp"

using namespace std;
using namespace util;

#define CELL_UPDATE_DEBUG_LOGGING

int main(int argc, char** argv) {
    int rows = 1000;
    int columns = rows;

    int iterations = 1000;

    bool useInitializerList = false;

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

    if (rows == 5 && columns == 5 && iterations == 1) {
        useInitializerList = true;
    }
    else if (rows == 5 && columns == 5) {
        cout << "size is 5 but iterations is not 1 so initializer list will not be used";
    }


    int printCount = max(iterations / 10, 1);

    chrono::time_point<chrono::system_clock> start, end;

//    ICellMatrix matrix = CellMatrix(rows, columns, maxOffset);
    MultiArrayCellMatrix matrix = MultiArrayCellMatrix(rows, columns, maxOffset);

    int numArrays = 2;
    int border = 1;

    uint8_t*** _arrays = new uint8_t**[numArrays];

    for (int i = 0; i < numArrays; i++) {
        _arrays[i] = LibraryCode::allocateArray(rows + (2 * border), columns + (2 * border));

        // Create teh border
        for (int row = 0; row < rows; row++) {
            for (int colInset = 0; colInset < border; colInset++) {
                _arrays[i][row][colInset] = 0;
                _arrays[i][row][columns - colInset] = 0;
            }
        }

        for (int col = 0; col < columns; col++) {
            for (int rowInset = 0; rowInset < border; rowInset++) {
                _arrays[i][rowInset][col] = 0;
                _arrays[i][rows - rowInset][col] = 0;
            }
        }
    }

    matrix.fillWithRandom();


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
        matrix.fillFromVector(initializer2);

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < columns; j++)
                _arrays[0][i+1][j+1] = initializer2.at(columns * i + j);
        }
    }



    const int sum = matrix.getSum();


    if (rows*columns < printThreshold * printThreshold) {
        cout << matrix << endl;
    }

//    ThreadPool threadPool(numThreads);

    bool updateOccurred = true;

    start = chrono::system_clock::now();

//    auto groups = calculateRowGroups(matrix, numThreads);

    for (int i = 0; i < iterations; i++) {
        // times for 1000x1000
        // goal: 2.6


        const int nextOffset = matrix.getNextOffset();
        const int offset  = matrix.getOffset();

        for (int row = 1; row < rows + 1; row++) {
            for (int column = 1; column < columns + 1; column++) {

                switch (
                        _arrays[offset][row - 1][column - 1] + _arrays[offset][row - 1][column] + _arrays[offset][row - 1][column + 1]
                        + _arrays[offset][row][column - 1] + _arrays[offset][row][column + 1]
                        + _arrays[offset][row + 1][column - 1] + _arrays[offset][row + 1][column] + _arrays[offset][row + 1][column + 1]
                        ) {
                    case (2):
//                        matrix.set(row, column, matrix.get(row, column), nextOffset);
                        _arrays[nextOffset][row][column] = _arrays[offset][row][column];
                        continue;
                        break;
                    case (3):
//                        matrix.set(row, column, true, nextOffset);
                        _arrays[nextOffset][row][column] = true;
                        continue;
                        break;
                    default:
//                        matrix.set(row, column, false, nextOffset);
                        _arrays[nextOffset][row][column] = false;
                        continue;
                        break;
                }


//                int neighborsAlive = 0;
//
//                #ifdef CELL_UPDATE_DEBUG_LOGGING
//                std::cout << "[" << std::endl;
//                #endif
//
//                for (int i = -1; i <= 1; i++) {
//                #ifdef CELL_UPDATE_DEBUG_LOGGING
//                    std::cout << "[";
//                #endif
//
//                    for (int j = -1; j <=1; j++) {
//                #ifdef CELL_UPDATE_DEBUG_LOGGING
//                        std::cout << " " << _arrays[offset][row + i][column + j];
//                #endif
//                        if (i != 0 || j != 0) {
//                            neighborsAlive += _arrays[offset][row + i][column + j];
//                        }
//                    }
//                #ifdef CELL_UPDATE_DEBUG_LOGGING
//                    std::cout << "]" << std::endl;
//                #endif
//
//                    if (neighborsAlive >= 4) {
//                        break;
//                    }
//                }
            }

        }

        matrix.incrementOffset();


//        if (i == printCount * multiplier) {
//            cout << "On iteration: " << i << " , " << (i/static_cast<double>(iterations))*100.0  << "%" << endl;
//              multiplier++;
//        }
//
//        if (!updateOccurred) {
//            cout << "exiting early because no update" << endl;
//            break;
//        }
    }

    end = chrono::system_clock::now();

    long seconds = chrono::duration_cast<chrono::seconds>(end - start).count();
    long ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    long decimal = ms - seconds * 1000;
    cout << "System took: " << seconds << "." << decimal << " seconds to run" << endl;

    if (rows*columns < printThreshold*printThreshold) {
        cout << "end matrix " << endl;
        std::stringstream msg("");
        msg << "[" << endl;
        for (int i = 0; i < rows; i++) {
            msg << "[ ";
            for (int j = 0; j < columns; j++) {
                msg << (_arrays[matrix.getOffset()][i + 1][j + 1] == 1 ? 1 : 0) << " ";
            }
            msg << "]" << endl;
        }
        msg << "]" << endl;

        cout << msg.str() << endl;
    }

    if (useInitializerList) {
        if (matrix.rows() * matrix.columns() < printThreshold * printThreshold) {
            bool success = true;
            const int offset = matrix.getOffset();

            for (int i = 0; i < matrix.rows(); i++) {
                for (int j = 0; j < matrix.columns(); j++) {
                    if (test2.at((i * rows) + j) != _arrays[offset][i + 1][j + 1]) {
                        success = false;
                        break;
                    }
                }
            }

            cout << "Success2: " << boolalpha << success << endl;
        }
    }


    for (int i = 0; i < maxOffset; i++) {
        LibraryCode::deleteArray(_arrays[i]);

        delete[] _arrays;
    }

    const double percent = (sum / static_cast<double>(rows*columns)) * 100.0;
    cout << "percent: " << percent << "%" << endl;

    return 0;
}