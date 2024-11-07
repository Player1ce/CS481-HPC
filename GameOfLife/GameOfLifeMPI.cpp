//
// Created by motst on 11/5/2024.
//
#include "../util/LibraryCode.hpp"
#include "../util/FileIO.hpp"

#ifdef _OPENMP
# include <omp.h>
#endif

#include <mpi.h>

#include <random>
#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <atomic>

//#include <mpi.h>

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
void fillWithRandom(T** array, const int rows, const int columns, const int border, const int min = 0, const int max = 1, const bool useRandom = true)
{
    // Create a random number generator

    if (useRandom) {
        std::random_device seed;
        std::mt19937 generator(seed());

        // Create a distribution for your desired range
        std::uniform_int_distribution<int> distribution(min, max);

        for (int i = border; i < rows + border; i++) {
            for (int j = border; j < columns + border; j++)
                array[i][j] = distribution(generator);
        }
    }
    else {
        std::mt19937 generator(12345);

        // Create a distribution for your desired range
        std::uniform_int_distribution<int> distribution(min, max);

        for (int i = border; i < rows + border; i++) {
            for (int j = border; j < columns + border; j++)
                array[i][j] = distribution(generator);
        }
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
    msg << "[\n";
    for (int i = border; i < rows + border; i++) {
        msg << "[ ";
        for (int j = border; j < columns + border; j++) {
            msg << array[i][j] << " ";
        }
        msg << "]\n";
    }
    msg << "]\n";
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

    constexpr bool useRandom = false;

    bool writeToFile = false;
    std::string outputDirectory;

    bool useTestFile = false;
    std::string testFile;

    int numThreads = 5;

    if (argc < 2) {
        cout << "Using coded constants" << endl;
    }
    else if (argc == 2) {
        cout << "Using Vector initializer" << endl;
        useInitializerList = true;
        rows = columns = 5;
        iterations = 1;
        numThreads = 1;
    }
    else if (argc == 3) {
        cout << "Using size: " << argv[1] << " and iterations: " << argv[2] << endl;
        rows = atoi(argv[1]);
        columns = rows;
        iterations = atoi(argv[2]);
    }
    else if (argc == 4) {
        cout << "Using size: " << argv[1] << " and iterations: " << argv[2] << " and numThreads: " << argv[3] << endl;
        rows = atoi(argv[1]);
        columns = rows;
        iterations = atoi(argv[2]);
        numThreads = atoi(argv[3]);

    }
    else if (argc == 5) {
        cout << "Using rows: " << argv[1] << " and iterations: " << argv[2] << " and numThreads: " << argv[3] << " and filePath: " << argv[4] << std::endl;
        rows = atoi(argv[1]);
        columns = rows;
        iterations = atoi(argv[2]);
        numThreads = atoi(argv[3]);
        outputDirectory = argv[4];
        writeToFile = true;
    }
    else if (argc == 6) {
        cout << "Using rows: " << argv[1] << " and iterations: " << argv[2] << " and numThreads: " << argv[3] << " and filePath: " << argv[4] << " and test file name:" << argv[5] << std::endl;
        rows = atoi(argv[1]);
        columns = rows;
        iterations = atoi(argv[2]);
        numThreads = atoi(argv[3]);
        outputDirectory = argv[4];
        writeToFile = true;

        testFile = argv[5];
        useTestFile = true;
    }

    if (rows == columns && columns == 5) {
        useInitializerList = true;
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
    fillWithRandom(_arrays[0], rows, columns, border, 0, 1, useRandom);

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

//#define EARLY_STOP_LOGGING

//#define SINGLE_THREAD_MODE
//#define OMP_MODE
#define MPI_MODE

//#define STANDARD_CHECK
//#define STANDARD_CHECK_OMP
//#define STANDARD_CHECK_OMP_TEST
#define STANDARD_CHECK_MPI

    start = chrono::system_clock::now();

//    ThreadPool threadPool(numThreads);

#if defined(OMP_MODE) || defined(MPI_MODE)
    auto groups = LibraryCode::calculateRowGroups(rows, numThreads);
    int groups_size = groups.size();
    numThreads = (groups_size < numThreads ? groups_size : numThreads);
#endif

//    constexpr int tracker_size = 3;

#ifdef SINGLE_THREAD_MODE
    int currentIteration = 0;
    for (currentIteration = 0; currentIteration < iterations; currentIteration++) {
        // standard_no_check 0.5
        // standard_check 0.8
        // standard_no_check_omp 0.55
        // standard_check_omp: 0.87

        // laptop
        // standard_nocheck: .935
        // standard_check_omp_test: 0.89

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
                int newVal = (value == 3) ? 1 : (value == 2) ? oldVal : 0;

                _arrays[nextOffset][row][column] = newVal;
                colsNoUpdates += oldVal == newVal;

            }
            rowsNoUpdates += colsNoUpdates == columns;
            colsNoUpdates = 0;
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
                    int newVal = (value == 3) ? 1 : (value == 2) ? oldVal : 0;


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
#endif

#ifdef OMP_MODE
    // region standard_check_omp_test
    #ifdef STANDARD_CHECK_OMP_TEST

    bool exit = false;
    atomic<int> atomicRowsNoUpdates = 0;

    #pragma omp parallel num_threads(numThreads) \
            default(none) \
            shared(_arrays, rows, columns, offset, nextOffset, groups, cout, iterations, exit, atomicRowsNoUpdates)
    {
        int my_rank;

        #ifdef _OPENMP
        my_rank = omp_get_thread_num();
//        cout << "my rank: " << my_rank << endl;
        #else
        my_rank = 0;
        #endif

        for (int currentIteration = 0; currentIteration < iterations && !exit; currentIteration++) {

            int innerRowsNoUpdates = 0;
            int innerColsNoUpdates = 0;

            //TODO: this does not work efficiently if border is not 1 because the overlap between rows will be more than 1 and things will recompute other things
            for (int row = groups.at(my_rank).first + border; row < groups.at(my_rank).second + border; row++) {
                for (int column = border; column < columns + border; column++) {

                    int value = _arrays[offset][row - 1][column - 1] + _arrays[offset][row - 1][column] +
                                _arrays[offset][row - 1][column + 1]
                                + _arrays[offset][row][column - 1] + _arrays[offset][row][column + 1]
                                + _arrays[offset][row + 1][column - 1] + _arrays[offset][row + 1][column] +
                                _arrays[offset][row + 1][column + 1];

                    int oldVal = _arrays[offset][row][column];
                    int newVal = (value == 3) ? 1 : (value == 2) ? oldVal : 0;

                    _arrays[nextOffset][row][column] = newVal;
                    innerColsNoUpdates += (oldVal == newVal);

//                    cout << "[" << row << ", " << column << ", s:" << sum << "] ";
                }
                innerRowsNoUpdates += (innerColsNoUpdates == columns);
                innerColsNoUpdates = 0;
//                cout << endl;
            }

            atomicRowsNoUpdates += innerRowsNoUpdates;

            #pragma omp barrier

            #pragma omp single
            {
                #ifdef EARLY_STOP_LOGGING
                cout << "Iteration: " << currentIteration + 1 << ", rows without updates: " << atomicRowsNoUpdates << endl;
                #endif

                offset = nextOffset;
                nextOffset = (offset + 1) % (maxOffset + 1);

                if (atomicRowsNoUpdates == rows) {
                    cout << "Exiting early on iteration: " << currentIteration + 1 << " because there was no update"
                         << endl;
                    exit = true;
                }

                #ifdef EARLY_STOP_LOGGING
                cout << arrayToString(_arrays[offset],  rows, columns, border) << endl;
                #endif

                atomicRowsNoUpdates = 0;
            }


        }
    }
#endif
    // endregion
#endif

#ifdef MPI_MODE

    // region STANDARD_CHECK_MPI
    #ifdef STANDARD_CHECK_MPI
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


    int *sendCounts = new int[groups.size()];
    int *displacements = new int[groups.size()];
    int local_sum = 0;

    int overlapBorder = 1;
    int overlap = 2*overlapBorder;

    int columnsToSend = columns - (2 * border);

    // TODO: the array needs to be formated as a single block of contiguous memory I think

    for (int i = 0; i < groups.size(); i++) {
        int rowsToSend = groups.at(i).second - groups.at(i).first + overlap;

        sendCounts[i] = (rowsToSend * (columns - 2 * border));
        local_sum += sendCounts[i] - overlap;
        displacements[i] = local_sum - overlapBorder;
    }

    int* sendBuffer = LibraryCode::convert2Dto1DArray<int>(_arrays[0], rows, columns, border);


    int recvCount = sendCounts[my_rank];
    int *receiveBuffer = new int[recvCount];


    MPI_Scatterv(sendBuffer, sendCounts, displacements, MPI_INT, receiveBuffer, recvCount, MPI_INT, 0, MPI_COMM_WORLD);

    int numRowsReceived = groups.at(my_rank).second - groups.at(my_rank).first + overlap;
    int numColsReceived = columnsToSend;

    int*** local_arrays = new int**[2];

    local_arrays[0] = LibraryCode::convert1Dto2DArray<int>(receiveBuffer, numRowsReceived, numColsReceived, 0, overlapBorder);

    for (int i = 1; i < numArrays; i++) {
        local_arrays[i] = LibraryCode::allocateArray<int>(numRowsReceived, columns + (2 * border));

        // Initialize the side borders only
        for (int row = 0; row < numRowsReceived; row++) {
            for (int colInset = 0; colInset < border; colInset++) {
                _arrays[i][row][colInset] = 0;
                _arrays[i][row][columns - colInset] = 0;
            }
        }
    }

    bool exit = false;


    for (int currentIteration = 0; currentIteration < iterations && !exit; currentIteration++) {

        rowsNoUpdates = 0;
        colsNoUpdates = 0;

        for (int row = overlapBorder; row < numRowsReceived - overlapBorder; row++) {
            for (int column = border; column < columns + border; column++) {

                int value = local_arrays[offset][row - 1][column - 1] + local_arrays[offset][row - 1][column] +
                            local_arrays[offset][row - 1][column + 1]
                            + local_arrays[offset][row][column - 1] + local_arrays[offset][row][column + 1]
                            + local_arrays[offset][row + 1][column - 1] + local_arrays[offset][row + 1][column] +
                            local_arrays[offset][row + 1][column + 1];

                int oldVal = local_arrays[offset][row][column];
                int newVal = (value == 3) ? 1 : (value == 2) ? oldVal : 0;

                local_arrays[nextOffset][row][column] = newVal;
                colsNoUpdates += (oldVal == newVal);

//                    cout << "[" << row << ", " << column << ", s:" << sum << "] ";
            }
            rowsNoUpdates += (colsNoUpdates == columns);
            colsNoUpdates = 0;
//                cout << endl;
        }

        //TODO: send edges here

        offset = nextOffset;
        nextOffset = (offset + 1) % (maxOffset + 1);

        // TODO: update this
        MPI_Barrier(MPI_COMM_WORLD);

        if (my_rank == 0) {

            #ifdef EARLY_STOP_LOGGING
            cout << "Iteration: " << currentIteration + 1 << ", rows without updates: " << atomicRowsNoUpdates << endl;
            #endif


            if (rowsNoUpdates == numRowsReceived - overlap) {
                // TODO: Send no change signal here and wait for response
                cout << "Exiting early on iteration: " << currentIteration + 1 << " because there was no update"
                     << endl;
                exit = true;
            }

            #ifdef EARLY_STOP_LOGGING
            cout << arrayToString(local_arrays[offset],  rows, columns, border) << endl;
            #endif
        }
    }

    LibraryCode::convert2Dto1DArray_inPlace(local_arrays[offset], receiveBuffer, numRowsReceived, columns + border * 2, 1);

    MPI_Gatherv(receiveBuffer, sendCounts[my_rank], MPI_INT, sendBuffer, sendCounts, displacements, MPI_INT, 0, MPI_COMM_WORLD);

    LibraryCode::convert1Dto2DArray_inPlace(sendBuffer, _arrays[offset], rows, columns, 0 ,1);

    // Finalize the MPI environment. No more MPI calls can be made after this
    delete[] sendCounts;
    delete[] displacements;
    delete[] receiveBuffer;

    MPI_Finalize();

    #endif
    // endregion

#endif

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
                    }
                }
                std::cout << std::endl;
            }

            cout << "Success: " << boolalpha << success << endl;
        }
    }

    if (writeToFile) {
        if (outputDirectory.back() != '/') {
            outputDirectory.append("/");
        }

        std::stringstream fileName;
        fileName << "output_" << rows << "x" << columns << "_" << iterations << "_";

        int fileNum = 0;

        auto filesInDirectory = file_io::listDirectory(outputDirectory);

//        cout << "Files in dir: ";
//        for (const auto& file : filesInDirectory) {
//            cout << file << " | ";
//        }
//        cout << endl;

//        cout << "test: " << outputDirectory << fileName.str() + std::to_string(fileNum) + ".txt" << endl;

        while (std::find(filesInDirectory.begin(), filesInDirectory.end(),
                         outputDirectory + fileName.str() + std::to_string(fileNum) + ".txt") != filesInDirectory.end()) {
            fileNum++;
        }

        fileName << fileNum << ".txt";

        if (file_io::writeTofile(outputDirectory + fileName.str(),
                                 {arrayToString(_arrays[offset], rows, columns, border)})) {
            cout << "successfully wrote output to file: " << outputDirectory << fileName.str() << endl;
        } else {
            cout << "Failed to write to file: " << outputDirectory << fileName.str() << endl;
        }
    }

    if (useTestFile) {
        std::string fileContents = file_io::readFullFile(outputDirectory + testFile);
//        cout << "file contents: " << endl << fileContents << endl;
//        cout << "result of to string: " << endl << arrayToString(_arrays[offset], rows, columns, border) << endl;
        bool success = fileContents == arrayToString(_arrays[offset], rows, columns, border) + "\n";
        cout << "Test file was the same: " << success << endl;
    }

    for (int i = 0; i < maxOffset; i++) {
        LibraryCode::deleteArray(_arrays[i]);

        delete[] _arrays;
    }

    const double percent = (sum / static_cast<double>(rows*columns)) * 100.0;
    cout << "percent: " << percent << "%" << endl;

    return 0;
}