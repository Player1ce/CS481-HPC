//
// Created by motst on 10/3/2024.
//

#include "../util/MultiArrayCellMatrix.hpp"
#include "../util/FixedSizeQueue.hpp"
#include "UpdateMethods.hpp"

#include <atomic>

#ifdef _OPENMP
# include <omp.h>
#endif

using namespace std;
using namespace util;

#define CELL_UPDATE_DEBUG_LOGGING

template<typename T>
void fillFromVector(T** array, const int rows, const int columns, const vector<bool> & list, const int border) {
    for (int i = border; i < rows + border; i++) {
        for (int j = border; j < columns + border; j++)
            array[i][j] = list.at(columns * (i - border) + (j - border));
    }
}

template<typename T>
void fillWithRandom(T** array, const int rows, const int columns, const int border, const int min = 0, const int max = 1)
{
    // Create a random number generator
    std::random_device seed;
    std::mt19937 generator(seed());

    // Create a distribution for your desired range
    std::uniform_int_distribution<int> distribution(min, max);

    for (int i = border; i < rows + border; i++) {
        for (int j = border; j < columns + border; j++)
            array[i][j] = distribution(generator);
    }
}

template <typename T>
int getSum(T** array, const int rows, const int columns, const int border) {
    int sum = 0;
    for (int i = border; i < rows + border; i++) {
        for (int j = border; j < columns + border; j++) {
            sum += array[i][j];
        }
    }
    return sum;
}

template<typename T>
std::string arrayToString(T** array, const int rows, const int columns, const int border ) {
    std::stringstream msg("");
    msg << "[" << endl;
    for (int i = border; i < rows + border; i++) {
        msg << "[ ";
        for (int j = border; j < columns + border; j++) {
            msg << array[i][j] << " ";
        }
        msg << "]" << endl;
    }
    msg << "]" << endl;
    return msg.str();
}


vector<bool> initializer = {
        true, true, false, false, true,
        false, true, true, false, false,
        true, true, false, false, true,
        false, true, true, true, true,
        true, true, true, false, true
};

vector<bool> test = {
        true, true, true, false, false,
        false, false, true, true, false,
        true, false, false, false, true,
        false, false, false, false, true,
        true, false, false, false, true,
};

vector<bool> initializer2 = {
        false, false, false, false, false,
        false, false, false, false, false,
        false, false, true, false, false,
        false, false, false, false, false,
        false, false, false, false, false
};

auto tester2 = {
        false, false, false, false, false,
        false, false, false, false, false,
        false, false, false, false, false,
        false, false, false, false, false,
        false, false, false, false, false
};
// theirs
//5000x1000: 46.3

// standalone checked
// 5000x1000: 25.7, 24.6


// standalone 10 thread checked
// 5000x1000: 11.4
// 5000x5000: 54, 57.185
// 10000x10000: 451.659

int main(int argc, char** argv) {
//    initializer = initializer2;
//    test = tester2;

    int rows = 1000;
    int columns = rows;

    int iterations = 1000;

    bool useInitializerList = false;

    int offset = 0;
    int nextOffset = 1;
    constexpr int maxOffset = 1;

    constexpr int printThreshold = 50;

    constexpr int border = 1;
    constexpr int numArrays = 2;

    int numThreads = 1;

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
        columns = rows;
        iterations = atoi(argv[2]);
        numThreads = atoi(argv[3]);

    }
    else if (argc == 5) {
        cout << "Using rows: " << argv[1] << " and columns: " << argv[2] << " and iterations: " << argv[3] << " and numThreads: " << argv[4] << endl;
        rows = atoi(argv[1]);
        columns = atoi(argv[2]);
        iterations = atoi(argv[3]);
        numThreads = atoi(argv[4]);
    }

    int printCount = max(iterations / 10, 1);

    int*** _arrays = new int**[2];

    // create the arrays and their borders
    for (int i = 0; i < numArrays; i++) {
        _arrays[i] = LibraryCode::allocateArray<int>(rows + (2 * border), columns + (2 * border));

        // Create the border
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

    // fill with random
    fillWithRandom(_arrays[0], rows, columns, border);

    if (useInitializerList) {
        fillFromVector(_arrays[0], rows, columns, initializer, border);
    }

    const int sum = getSum(_arrays[0], rows, columns, border);

    if (rows*columns < printThreshold * printThreshold) {
        cout << arrayToString(_arrays[0], rows, columns, border) << endl;
    }

    int colsNoUpdates = 0;
    int rowsNoUpdates = 0;

    bool updateOccurred = true;


    chrono::time_point<chrono::system_clock> start, end;

//#define STANDARD_NO_CHECK
#define STANDARD_CHECK
//#define STANDARD_NO_CHECK_OMP
//#define STANDARD_CHECK_OMP
//#define WINDOWS

    start = chrono::system_clock::now();

//    ThreadPool threadPool(numThreads);

#if defined(STANDARD_NO_CHECK_OMP) || defined(STANDARD_CHECK_OMP)
    auto groups = calculateRowGroups(rows, numThreads);
    int groups_size = groups.size();
    numThreads = (groups_size < numThreads ? groups_size : numThreads);
#endif

//    constexpr int tracker_size = 3;
    int tracker[3];
    int currentIteration = 0;
    for (currentIteration = 0; currentIteration < iterations; currentIteration++) {
        // standard_no_check 0.5
        // standard_check 0.8
        // standard_no_check_omp 0.55
        // standard_check_omp: 0.87

        // region standard_no_check
        #ifdef STANDARD_NO_CHECK

        // mine: 0.5
        // theirs: 1.6

        for (int row = border; row < rows + border; row++) {
            for (int column = border; column < columns + border; column++) {
                int value = _arrays[offset][row - 1][column - 1] + _arrays[offset][row - 1][column] +
                            _arrays[offset][row - 1][column + 1]
                            + _arrays[offset][row][column - 1] + _arrays[offset][row][column + 1]
                            + _arrays[offset][row + 1][column - 1] + _arrays[offset][row + 1][column] +
                            _arrays[offset][row + 1][column + 1];

                int oldVal = _arrays[offset][row][column];
                int newVal = 0;

                if (value == 3) {
                    newVal = 1;
                }
                else if (value == 2) {
                    newVal = oldVal;
                }
                _arrays[nextOffset][row][column] = newVal;

//                if (_arrays[offset][row][column]) { // cell was alive in the earlier iteration
//                    if (value < 2 || value > 3) {
//                        _arrays[nextOffset][row][column] = 0;
//                    }
//                    else // value must be 2 or 3, so no need to check explicitly
//                        _arrays[nextOffset][row][column] = 1 ; // no change
//                }
//                else { // cell was dead in the earlier iteration
//                    if (value == 3) {
//                        _arrays[nextOffset][row][column] = 1;
//                    }
//                    else
//                        _arrays[nextOffset][row][column] = 0; // no change
//                }

//                switch (
//                        _arrays[offset][row - 1][column - 1] + _arrays[offset][row - 1][column] + _arrays[offset][row - 1][column + 1]
//                        + _arrays[offset][row][column - 1] + _arrays[offset][row][column + 1]
//                        + _arrays[offset][row + 1][column - 1] + _arrays[offset][row + 1][column] + _arrays[offset][row + 1][column + 1]
//                        ) {
//                    case (2):
//                        _arrays[nextOffset][row][column] = _arrays[offset][row][column];
//                        continue;
//                        break;
//                    case (3):
//                        _arrays[nextOffset][row][column] = true;
//                        continue;
//                        break;
//                    default:
//                        _arrays[nextOffset][row][column] = false;
//                        continue;
//                        break;
//                }
            }

        }

        #endif
        // endregion

        // region standard_check
        #ifdef STANDARD_CHECK
        updateOccurred = false;
        colsNoUpdates = 0;
        rowsNoUpdates = 0;

        // mine (full check): 0.8
        // mine (every time):
        // theirs: 1.8 with bool;  1.9 with flag
        //
        // method 1updateOccurred = (oldVal != newVal) || updateOccurred;: 2.5
        // method 2if (!updateOccurred && oldVal != newVal) updateOccurred = true;: 2.3
        // flag method: 2.15
        // flag += oldVal != newVal; : 2.2
        //

        for (int row = border; row < rows + border; row++) {
            for (int column = border; column < columns + border; column++) {

                int value = _arrays[offset][row - 1][column - 1] + _arrays[offset][row - 1][column] +
                            _arrays[offset][row - 1][column + 1]
                            + _arrays[offset][row][column - 1] + _arrays[offset][row][column + 1]
                            + _arrays[offset][row + 1][column - 1] + _arrays[offset][row + 1][column] +
                            _arrays[offset][row + 1][column + 1];

                int oldVal = _arrays[offset][row][column];
                int newVal = 0;

                if (value == 3) {
                    newVal = 1;

                }
                else if (value == 2) {
                    newVal = oldVal;
                }

                _arrays[nextOffset][row][column] = newVal;
                colsNoUpdates += oldVal == newVal;

//                if (_arrays[offset][row][column]) { // cell was alive in the earlier iteration
//                    if (value < 2 || value > 3) {
//                        _arrays[nextOffset][row][column] = 0;
//                        flag++; // value changed
//                    }
//                    else // value must be 2 or 3, so no need to check explicitly
//                        _arrays[nextOffset][row][column] = 1 ; // no change
//                }
//                else { // cell was dead in the earlier iteration
//                    if (value == 3) {
//                        _arrays[nextOffset][row][column] = 1;
//                        flag++; // value changed
//                    }
//                    else
//                        _arrays[nextOffset][row][column] = 0; // no change
//                }
            }
            rowsNoUpdates += colsNoUpdates == columns;
            colsNoUpdates = 0;
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

            for (int row = groups.at(my_rank).first + border; row < groups.at(my_rank).second + border; row++) {
                for (int column = border; column < columns + border; column++) {

                    int value = _arrays[offset][row - 1][column - 1] + _arrays[offset][row - 1][column] +
                                _arrays[offset][row - 1][column + 1]
                                + _arrays[offset][row][column - 1] + _arrays[offset][row][column + 1]
                                + _arrays[offset][row + 1][column - 1] + _arrays[offset][row + 1][column] +
                                _arrays[offset][row + 1][column + 1];

                    int oldVal = _arrays[offset][row][column];
                    int newVal = 0;

                    if (value == 3) {
                        newVal = 1;

                    }
                    else if (value == 2) {
                        newVal = oldVal;
                    }

                    _arrays[nextOffset][row][column] = newVal;
                }
//                cout << endl;
            }
        }

        #endif
        // endregion

        // region standard_check_omp
        #ifdef STANDARD_CHECK_OMP
        rowsNoUpdates = 0;

        #pragma omp parallel num_threads(numThreads) \
            default(none) \
            shared(_arrays, rows, columns, offset, nextOffset, groups, rowsNoUpdates, cout)

        {
            int my_rank;
            int innerRowsNoUpdates = 0;
            int innerColsNoUpdates = 0;

            #ifdef _OPENMP
            my_rank = omp_get_thread_num();
//            cout << "my rank: " << my_rank << endl;
            #else
            my_rank = 0;
            #endif

//            this_thread::sleep_for(chrono::milliseconds(my_rank * 100));

            for (int row = groups.at(my_rank).first + border; row < groups.at(my_rank).second + border; row++) {
                for (int column = border; column < columns + border; column++) {

                    int value = _arrays[offset][row - 1][column - 1] + _arrays[offset][row - 1][column] +
                              _arrays[offset][row - 1][column + 1]
                              + _arrays[offset][row][column - 1] + _arrays[offset][row][column + 1]
                              + _arrays[offset][row + 1][column - 1] + _arrays[offset][row + 1][column] +
                              _arrays[offset][row + 1][column + 1];

                    int oldVal = _arrays[offset][row][column];
                    int newVal = 0;

                    if (value == 3) {
                        newVal = 1;

                    }
                    else if (value == 2) {
                        newVal = oldVal;
                    }

                    _arrays[nextOffset][row][column] = newVal;
                    innerColsNoUpdates += oldVal == newVal;

//                    cout << "[" << row << ", " << column << ", s:" << sum << "] ";
                }
                innerRowsNoUpdates += innerColsNoUpdates == columns;
                innerColsNoUpdates = 0;
//                cout << endl;
            }

            #pragma omp critical
            {
                rowsNoUpdates += innerRowsNoUpdates;
            }
        }

        #endif
        // endregion

        // region windows
        #ifdef WINDOWS
        for (int row = border; row < rows+border; row++) {

//            windowTracker.resetQueue();

            tracker[0] = 0;
            tracker[1] = 0;
            tracker[2] = 0;

            int index = 1;
            int tracker_sum = _arrays[offset][row - 1][border] +
                              _arrays[offset][row]    [border] +
                              _arrays[offset][row + 1][border];

            tracker[0] = tracker_sum;


//            std::cout << std::endl;

            for (int col = border + 1; col < columns+border; col++) {

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

            int current_val = _arrays[offset][row][columns + (border - 1)];
            int adjusted_sum = tracker_sum - current_val;


            _arrays[nextOffset][row][columns + (border - 1)] = (adjusted_sum == 2) ? current_val : (adjusted_sum == 3);

//            switch (tracker_sum - current_val) {
//                case (2):
//                    _arrays[nextOffset][row][columns + (border - 1)] = current_val;
//                    continue;
//                    break;
//                case (3):
//                    _arrays[nextOffset][row][columns + (border - 1)] = true;
//                    continue;
//                    break;
//                default:
//                    _arrays[nextOffset][row][columns + (border - 1)] = false;
//                    continue;
//                    break;
//            }
        }
//        std::cout << std::endl;


        #endif
        // endregion

        offset = nextOffset;
        nextOffset = (offset + 1) % (maxOffset + 1);

        if (rowsNoUpdates == rows) {
            cout << "exiting early on iteration: " << currentIteration + 1 << " because there was no update" << endl;
            break;
        }

//        if (i == printCount * multiplier) {
//            cout << "On iteration: " << currentIteration << " , " << (i/static_cast<double>(iterations))*100.0  << "%" << endl;
//              multiplier++;
//        }
    }

    end = chrono::system_clock::now();

    long seconds = chrono::duration_cast<chrono::seconds>(end - start).count();
    long ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    long decimal = ms - seconds * 1000;
    cout << "System took: " << seconds << "." << decimal << " seconds to run" << endl;

    if (rows*columns < printThreshold*printThreshold) {
        cout << "end matrix " << endl << arrayToString(_arrays[offset], rows, columns, border);
    }

    if (useInitializerList) {
        if (rows * columns < printThreshold * printThreshold) {
            bool success = true;

            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < columns; j++) {
                    cout << "[t:" << test.at((i * rows) + j) << ", r:" << (_arrays[offset][i + border][j + border] == 1 ? 1 : 0) << "] ";
                    if (test.at((i * rows) + j) != (_arrays[offset][i + border][j + border] == 1 ? 1 : 0)) {
                        success = false;
                        break;
                    }
                }
                std::cout << std::endl;
            }

            cout << "Success: " << boolalpha << success << endl;
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