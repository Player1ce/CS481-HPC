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

int main(int argc, char** argv) {
    int rows = 1000;
    int columns = rows;

    int iterations = 1000;

    bool useInitializerList = false;

    constexpr int maxOffset = 1;

    constexpr int printThreshold = 50;

    constexpr int border = 1;
    constexpr int numArrays = 2;

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

    MultiArrayCellMatrix matrix = MultiArrayCellMatrix(1, 1, maxOffset);

    int*** _arrays = new int**[2];
//    int** _arrays[2];

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
        fillFromVector(_arrays[0], rows, columns, initializer2, border);
    }

    const int sum = getSum(_arrays[0], rows, columns, border);


    if (rows*columns < printThreshold * printThreshold) {
        cout << arrayToString(_arrays[0], rows, columns, border) << endl;
    }

    bool updateOccurred = false;


    chrono::time_point<chrono::system_clock> start, end;

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

    for (int i = 0; i < iterations; i++) {
        // times for 1000x1000
        // goal: 2.6

        const int offset = matrix.getOffset();
        const int nextOffset = matrix.getNextOffset();


        // region standard_no_check
        #ifdef STANDARD_NO_CHECK

        for (int row = border; row < rows + border; row++) {
            for (int column = border; column < columns + border; column++) {

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

            for (int row = groups.at(my_rank).first + border; row < groups.at(my_rank).second + border; row++) {
                for (int column = border; column < columns + border; column++) {

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
        cout << "end matrix " << endl << arrayToString(_arrays[matrix.getOffset()], rows, columns, border);
    }

    if (useInitializerList) {
        if (matrix.rows() * matrix.columns() < printThreshold * printThreshold) {
            bool success = true;
            const int offset = matrix.getOffset();

            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < columns; j++) {
                    cout << "[t:" << test2.at((i*rows) + j) << ", r:" << (_arrays[offset][i + border][j + border] == 1 ? 1 : 0) << "] ";
                    if (test2.at((i * rows) + j) != (_arrays[offset][i + border][j + border] == 1 ? 1 : 0)) {
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

        delete[] _arrays;
    }

    const double percent = (sum / static_cast<double>(rows*columns)) * 100.0;
    cout << "percent: " << percent << "%" << endl;

    return 0;
}