//
// Created by motst on 10/3/2024.
//

#include "../util/MultiArrayCellMatrix.hpp"
#include "../util/FixedSizeQueue.hpp"
#include "UpdateMethods.hpp"

#ifdef _OPENMP
# include <omp.h>
#endif

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

    int numThreads = 6;

    if (argc < 2) {
        cout << "Using coded constants" << endl;
    }
    else if (argc == 2) {
        cout << "Using Vector initializer" << endl;
        useInitializerList = true;
        rows = columns = 5;
        iterations = 1;
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

    MultiArrayCellMatrix matrix = MultiArrayCellMatrix(1, 1, maxOffset);

    constexpr int numArrays = 2;
    int border = 1;

//    int*** _arrays = new int**[2];
    int** _arrays[2];

    for (int i = 0; i < numArrays; i++) {
        _arrays[i] = LibraryCode::allocateArray<int>(rows + (2 * border), columns + (2 * border));

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

    bool updateOccurred = true;


#define STANDARD_NO_CHECK
//#define STANDARD_NO_CHECK_OMP
//#define WINDOWS

    start = chrono::system_clock::now();

//    ThreadPool threadPool(numThreads);

#ifdef STANDARD_NO_CHECK_OMP
    auto groups = calculateRowGroups(matrix, numThreads);
    int groups_size = groups.size();
    numThreads = (groups_size < numThreads ? groups_size : numThreads);
#endif

//    constexpr int tracker_size = 3;
    int tracker[3];

//    int index = 0, tracker_sum = 0;



    for (int i = 0; i < iterations; i++) {
        // times for 1000x1000
        // goal: 2.6

        const auto offset = matrix.getOffset();
        const auto nextOffset = matrix.getNextOffset();


        // region standard_no_check
        #ifdef STANDARD_NO_CHECK

        for (int row = 1; row < rows + 1; row++) {
            for (int column = 1; column < columns + 1; column++) {

                switch (
                        _arrays[offset][row - 1][column - 1] + _arrays[offset][row - 1][column] + _arrays[offset][row - 1][column + 1]
                        + _arrays[offset][row][column - 1] + _arrays[offset][row][column + 1]
                        + _arrays[offset][row + 1][column - 1] + _arrays[offset][row + 1][column] + _arrays[offset][row + 1][column + 1]
                        ) {
                    case (2):
                        _arrays[nextOffset][row][column] = _arrays[offset][row][column];
                        continue;
                        break;
                    case (3):
                        _arrays[nextOffset][row][column] = true;
                        continue;
                        break;
                    default:
                        _arrays[nextOffset][row][column] = false;
                        continue;
                        break;
                }
            }

        }

        #endif
        // endregion


        // region standard_no_check_omp
        #ifdef STANDARD_NO_CHECK_OMP

        #pragma omp parallel num_threads(numThreads) \
            default(none) \
            shared(_arrays, rows, columns, offset, nextOffset, groups, cout)
        {


            int my_rank;
            #ifdef _OPENMP
            my_rank = omp_get_thread_num();
            #else
            my_rank = 0;
            #endif

//            this_thread::sleep_for(chrono::milliseconds(my_rank * 100));

            for (int row = groups.at(my_rank).first + 1; row < groups.at(my_rank).second + 1; row++) {
                for (int column = 1; column < columns + 1; column++) {

//                    int sum = _arrays[offset][row - 1][column - 1] + _arrays[offset][row - 1][column] +
//                              _arrays[offset][row - 1][column + 1]
//                              + _arrays[offset][row][column - 1] + _arrays[offset][row][column + 1]
//                              + _arrays[offset][row + 1][column - 1] + _arrays[offset][row + 1][column] +
//                              _arrays[offset][row + 1][column + 1];
//
//                    cout << "[" << row << ", " << column << ", s:" << sum << "] ";

                    switch (
                            _arrays[offset][row - 1][column - 1] + _arrays[offset][row - 1][column] +
                            _arrays[offset][row - 1][column + 1]
                            + _arrays[offset][row][column - 1] + _arrays[offset][row][column + 1]
                            + _arrays[offset][row + 1][column - 1] + _arrays[offset][row + 1][column] +
                            _arrays[offset][row + 1][column + 1]
                            ) {
                        case (2):
                            _arrays[nextOffset][row][column] = _arrays[offset][row][column];
                            continue;
                            break;
                        case (3):
                            _arrays[nextOffset][row][column] = true;
                            continue;
                            break;
                        default:
                            _arrays[nextOffset][row][column] = false;
                            continue;
                            break;
                    }
                }
//                cout << endl;
            }
            #pragma omp barrier
        }

        #endif
        // endregion

        // region windows
        #ifdef WINDOWS
        for (int row = 1; row < rows+1; row++) {

//            windowTracker.resetQueue();

            tracker[0] = 0;
            tracker[1] = 0;
            tracker[2] = 0;

            int index = 1;
            int tracker_sum = _arrays[offset][row - 1][1] +
                              _arrays[offset][row]    [1] +
                              _arrays[offset][row + 1][1];

            tracker[0] = tracker_sum;


//            std::cout << std::endl;

            for (int col = 2; col < columns+1; col++) {

                // load in the current column (start at two because we write behind)
                tracker_sum -= tracker[index];

                tracker[index] = _arrays[offset][row - 1][col] +
                                 _arrays[offset][row    ][col] +
                                 _arrays[offset][row + 1][col];

                tracker_sum += tracker[index];

                index = (index + 1) % 3;

                int current_val = _arrays[offset][row][col-1];
                int adjusted_sum = tracker_sum - current_val;

                _arrays[nextOffset][row][col - 1] = (adjusted_sum == 2) ? current_val : (adjusted_sum == 3);

//                switch (tracker_sum - current_val) {
//                    case (2):
//                        _arrays[nextOffset][row][col-1] = current_val;
//                        continue;
//                        break;
//                    case (3):
//                        _arrays[nextOffset][row][col-1] = true;
//                        continue;
//                        break;
//                    default:
//                        _arrays[nextOffset][row][col-1] = false;
//                        continue;
//                        break;
//                }
            }


            tracker_sum -= tracker[index];

            tracker[index] = 0;

            int current_val = _arrays[offset][row][columns];
            int adjusted_sum = tracker_sum - current_val;


            _arrays[nextOffset][row][columns] = (adjusted_sum == 2) ? current_val : (adjusted_sum == 3);

//            switch (windowTracker.sum() - _arrays[offset][row][columns]) {

//            switch (tracker_sum - current_val) {
//                case (2):
//                    _arrays[nextOffset][row][columns] = current_val;
//                    continue;
//                    break;
//                case (3):
//                    _arrays[nextOffset][row][columns] = true;
//                    continue;
//                    break;
//                default:
//                    _arrays[nextOffset][row][columns] = false;
//                    continue;
//                    break;
//            }
        }
//        std::cout << std::endl;


        #endif
        // endregion

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
        for (int i = + 1; i < rows + 1; i++) {
            msg << "[ ";
            for (int j = 1; j < columns + 1; j++) {
                msg << (_arrays[matrix.getOffset()][i][j] == 1 ? 1 : 0) << " ";
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
                    cout << "[t:" << test2.at((i*rows) + j) << ", r:" << (_arrays[offset][i+1][j+1] == 1 ? 1 : 0) << "] ";
                    if (test2.at((i * rows) + j) != (_arrays[offset][i+1][j+1] == 1 ? 1 : 0)) {
                        success = false;
                        break;
                    }
                }
                std::cout << std::endl;
            }

            cout << "Success2: " << boolalpha << success << endl;
        }
    }


    for (int i = 0; i < maxOffset; i++) {
        LibraryCode::deleteArray(_arrays[i]);

//        delete[] _arrays;
    }

    const double percent = (sum / static_cast<double>(rows*columns)) * 100.0;
    cout << "percent: " << percent << "%" << endl;

    return 0;
}