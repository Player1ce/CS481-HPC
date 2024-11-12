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

using namespace std;
using namespace util;

#define CELL_UPDATE_DEBUG_LOGGING

template<typename T>
void fillFromVector(T **array, const int rows, const int columns, const vector<bool> &list, const int border) {
    for (int i = border; i < rows + border; i++) {
        for (int j = border; j < columns + border; j++)
            array[i][j] = list.at(columns * (i - border) + (j - border));
    }
}

template<typename T>
void fillWithRandom(T **array, const int rows, const int columns, const int border, const int min = 0,
                    const int max = 1, const bool useRandom = true) {
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
    } else {
        std::mt19937 generator(12345);

        // Create a distribution for your desired range
        std::uniform_int_distribution<int> distribution(min, max);

        for (int i = border; i < rows + border; i++) {
            for (int j = border; j < columns + border; j++)
                array[i][j] = distribution(generator);
        }
    }
}

template<typename T>
int getSum(T **array, const int rows, const int columns, const int border) {
    int sum = 0;
    for (int i = border; i < rows + border; i++) {
        for (int j = border; j < columns + border; j++) {
            sum += array[i][j];
        }
    }
    return sum;
}

template<typename T>
std::string arrayToString(T **array, const int rows, const int columns, const int border) {
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

vector<bool> test_secondIteration = {
    false, true, true, true, false,
    true, false, true, true, false,
    false, false, false, false, true,
    false, false, false, true, true,
    false, false, false, false, false,
};

vector<bool> initializer2 = {
    true, false, false, false, false,
    false, true, false, false, false,
    false, false, true, false, false,
    false, false, false, true, false,
    false, false, false, false, true
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


//#define EARLY_STOP_LOGGING
// #define MPI_DEBUG_LOGGING

#define STANDARD_CHECK_MPI
// #define STANDARD_CHECK_MPI_NONBLOCKING

// Tests on 1000x1000 boards for 1000 iterations with 1 thread

// Laptop
// STANDARD_CHECK_MPI: 1.89

// Desktop
// STANDARD_CHECK_OMP_TEST: 0.84
// STANDARD_CHECK_MPI: 0.89


int main(int argc, char **argv) {
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // initialize variables ----------------------------------------
    // initializer = initializer2;
    // test = tester2;

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

    int numThreads = 1;

    int ***_arrays = nullptr;
    int sum;

    // use command line arguments ----------------------------------------------------------
    if (argc < 2) {
        cout << "Using coded constants" << endl;
    } else if (argc == 2) {
        if (my_rank == 0) cout << "Using Vector initializer" << endl;
        useInitializerList = true;
        rows = columns = 5;
        iterations = 1;
        numThreads = 1;
    } else if (argc == 3) {
        if (my_rank == 0) cout << "Using size: " << argv[1] << " and iterations: " << argv[2] << endl;
        rows = atoi(argv[1]);
        columns = rows;
        iterations = atoi(argv[2]);
    } else if (argc == 4) {
        if (my_rank == 0)
            cout << "Using size: " << argv[1] << " and iterations: " << argv[2] << " and numProcesses: " <<
                    argv[3] << endl;
        rows = atoi(argv[1]);
        columns = rows;
        iterations = atoi(argv[2]);
        numThreads = atoi(argv[3]);
    } else if (argc == 5) {
        if (my_rank == 0)
            cout << "Using rows: " << argv[1] << " and iterations: " << argv[2] << " and numProcesses: " <<
                    argv[3] << " and filePath: " << argv[4] << std::endl;
        rows = atoi(argv[1]);
        columns = rows;
        iterations = atoi(argv[2]);
        numThreads = atoi(argv[3]);
        outputDirectory = argv[4];
        writeToFile = true;
    } else if (argc == 6) {
        if (my_rank == 0)
            cout << "Using rows: " << argv[1] << " and iterations: " << argv[2] << " and numProcesses: " <<
                    argv[3] << " and filePath: " << argv[4] << " and test file name:" << argv[5] << std::endl;
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

        // special case to test for early stop
        if (iterations == 10) {
            initializer = initializer2;
            test = tester2;
        }
    }

    if (world_size > rows) {
        cout << "ERROR: cannot have more processes than rows in the grid." << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }

    int printCount = max(iterations / 10, 1);

    if (my_rank == 0) {
        _arrays = new int **[2];

        // create the arrays and their borders
        for (int i = 0; i < numArrays; i++) {
            _arrays[i] = LibraryCode::allocateArray<int>(rows + (2 * border), columns + (2 * border));

            // Create the border
            for (int row = 0; row < rows + 2 * border; row++) {
                for (int colInset = 0; colInset < border; colInset++) {
                    _arrays[i][row][colInset] = 0;
                    _arrays[i][row][columns + border - colInset] = 0;
                }
            }

            for (int col = 0; col < columns + 2 * border; col++) {
                for (int rowInset = 0; rowInset < border; rowInset++) {
                    _arrays[i][rowInset][col] = 0;
                    _arrays[i][rows + border - rowInset][col] = 0;
                }
            }
        }

        // fill with random
        fillWithRandom(_arrays[0], rows, columns, border, 0, 1, useRandom);

        if (useInitializerList) {
            fillFromVector(_arrays[0], rows, columns, initializer, border);
        }

        sum = getSum(_arrays[0], rows, columns, border);


        if (rows * columns < printThreshold * printThreshold) {
            cout << arrayToString(_arrays[0], rows, columns, border) << endl;
        }
    }


    int colsNoUpdates = 0;
    int rowsNoUpdates = 0;

    bool updateOccurred = true;

    MPI_Barrier(MPI_COMM_WORLD);

    chrono::time_point<chrono::system_clock> start, end;

    if (my_rank == 0) {
        start = chrono::system_clock::now();
    }

    auto groups = LibraryCode::calculateRowGroups(rows, world_size);

    #ifdef MPI_DEBUG_LOGGING
    if (my_rank == 0) {
        cout << "world_size: " << world_size << endl;
    }
    #endif

    // every process
    int *secondSendBuffer = nullptr;
    int *receiveBuffer = nullptr;

    int *rowSendBuffer = nullptr;
    int *rowReceiveBuffer = nullptr;

    // root only
    int *sendCounts = nullptr;
    int *displacements = nullptr;
    int *sendBuffer = nullptr;
    int *secondReceiveBuffer = nullptr;

    // region STANDARD_CHECK_MPI
    #ifdef STANDARD_CHECK_MPI

    int overlapBorder = 1;
    int overlap = 2 * overlapBorder;

    int columnsToSend = columns + 2 * border;
    int rowsToSend;

    int my_recvCount = 0;
    int my_sendCount = 0;

    if (my_rank == 0) {
        sendCounts = new int[groups.size()];
        displacements = new int[groups.size()];

        int local_sum = 0;
        for (int i = 0; i < groups.size(); i++) {
            rowsToSend = groups.at(i).second - groups.at(i).first + overlap;

            sendCounts[i] = ((rowsToSend) * columnsToSend);
            displacements[i] = local_sum;
            local_sum += sendCounts[i] - (overlap * columnsToSend);
        }

        rowsToSend = groups.at(0).second - groups.at(0).first + overlap;
        my_recvCount = sendCounts[0];
    } else {
        rowsToSend = groups.at(my_rank).second - groups.at(my_rank).first + overlap;

        my_recvCount = (rowsToSend * columnsToSend);
        my_sendCount = my_recvCount;
    }

    if (my_rank == 0) {
        sendBuffer = new int[(rows + 2 * border) * (columns + 2 * border)];

        for (int row = 0; row < rows + 2 * border; row++) {
            for (int col = 0; col < columns + 2 * border; col++) {
                sendBuffer[row * (columns + 2 * border) + col] = _arrays[0][row][col];
            }
        }

        #ifdef MPI_DEBUG_LOGGING
        cout << "print the sendBuffer" << endl;
        for (int i = 0; i < rows + 2 * border; i++) {
            cout << "[";
            for (int j = 0; j < columns + 2 * border; j++) {
                cout << sendBuffer[i * (columns + 2 * border) + j] << " ";
            }
            cout << "]" << endl;
        }
        #endif
    }

    receiveBuffer = new int[my_recvCount];

    MPI_Scatterv(sendBuffer, sendCounts, displacements, MPI_INT, receiveBuffer, my_recvCount, MPI_INT, 0,
                 MPI_COMM_WORLD);


    int numRowsReceived = groups.at(my_rank).second - groups.at(my_rank).first + overlap;
    #ifdef MPI_DEBUG_LOGGING
    cout << "Process " << my_rank << ": " << "numRowsReceived = " << numRowsReceived << endl;
    cout << "Process " << my_rank << ": " << "group first: " << groups.at(my_rank).first << ", group second: " << groups
            .at(my_rank).second << endl;
    #endif

    int numColsReceived = columnsToSend;

    #ifdef MPI_DEBUG_LOGGING
    cout << "Process " << my_rank << ": " << "numColsReceived = " << numColsReceived << endl;
    #endif

    rowSendBuffer = new int[numColsReceived];
    rowReceiveBuffer = new int[numColsReceived];

    for (int i = 0; i < numColsReceived; i++) {
        rowSendBuffer[i] = 0;
        rowReceiveBuffer[i] = 0;
    }

    int ***local_arrays = new int **[numArrays];

    // special case for first array, because borders are sent through mpi
    local_arrays[0] = LibraryCode::allocateArray<int>(numRowsReceived, numColsReceived);

    for (int i = 1; i < numArrays; i++) {
        local_arrays[i] = LibraryCode::allocateArray<int>(numRowsReceived, numColsReceived);

        // Initialize the borders
        for (int row = 0; row < numRowsReceived; row++) {
            for (int colInset = 0; colInset < border; colInset++) {
                local_arrays[i][row][colInset] = 0;
                local_arrays[i][row][numColsReceived - colInset - 1] = 0;
            }
        }

        for (int col = 0; col < numColsReceived; col++) {
            for (int rowInset = 0; rowInset < border; rowInset++) {
                local_arrays[i][rowInset][col] = 0;
                local_arrays[i][numRowsReceived - rowInset - 1][col] = 0;
            }
        }
    }

    // copy received data to local array
    for (int row = 0; row < numRowsReceived; row++) {
        for (int col = 0; col < numColsReceived; col++) {
            local_arrays[0][row][col] = receiveBuffer[(row * numColsReceived) + col];
        }
    }

    #ifdef MPI_DEBUG_LOGGING
    cout << "Process " << my_rank << ": " << "print local_arrays[0]" << endl;
    for (int i = 0; i < numRowsReceived; i++) {
        cout << "[";
        for (int j = 0; j < numColsReceived; j++) {
            cout << local_arrays[0][i][j] << " ";
        }
        cout << "]" << endl;
    }
    #endif

    int exit = 0;


    for (int currentIteration = 0; currentIteration < iterations && !exit; currentIteration++) {
        rowsNoUpdates = 0;
        colsNoUpdates = 0;

        for (int row = overlapBorder; row < numRowsReceived - overlapBorder; row++) {
            for (int column = border; column < numColsReceived - border; column++) {
                int value = local_arrays[offset][row - 1][column - 1] + local_arrays[offset][row - 1][column] +
                            local_arrays[offset][row - 1][column + 1]
                            + local_arrays[offset][row][column - 1] + local_arrays[offset][row][column + 1]
                            + local_arrays[offset][row + 1][column - 1] + local_arrays[offset][row + 1][column] +
                            local_arrays[offset][row + 1][column + 1];

                int oldVal = local_arrays[offset][row][column];
                int newVal = (value == 3) ? 1 : (value == 2) ? oldVal : 0;

                local_arrays[nextOffset][row][column] = newVal;
                colsNoUpdates += (oldVal == newVal);

                // cout << "[" << row << ", " << column << ", s:" << sum << "] ";
            }
            rowsNoUpdates += (colsNoUpdates == columns);
            colsNoUpdates = 0;
            // cout << endl;
        }

        // send Edges


        // exchange upper row
        if (my_rank != 0) {
            for (int col = 0; col < numColsReceived; col++) {
                rowSendBuffer[col] = local_arrays[nextOffset][1][col];
            }
            MPI_Sendrecv(rowSendBuffer, numColsReceived, MPI_INT, my_rank - 1, 0, rowReceiveBuffer, numColsReceived, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            #ifdef MPI_DEBUG_LOGGING
            cout << "Process " << my_rank << ": " << "print rowSendBuffer" << endl;
            cout << "[";
            for (int i = 0; i < numColsReceived; i++) {
                cout << " " << rowSendBuffer[i];
            }
            cout << "]" << endl;
            #endif

            for (int col = 0; col < numColsReceived; col++) {
                local_arrays[nextOffset][0][col] = rowReceiveBuffer[col];
            }

            // MPI_Send(rowSendBuffer, numColsReceived, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD);
        }

        // exchange lower row
        if (my_rank != world_size - 1) {
            for (int col = 0; col < numColsReceived; col++) {
                rowSendBuffer[col] = local_arrays[nextOffset][numRowsReceived - 2][col];
            }

            MPI_Sendrecv(rowSendBuffer, numColsReceived, MPI_INT, my_rank + 1, 0, rowReceiveBuffer, numColsReceived, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            #ifdef MPI_DEBUG_LOGGING
            cout << "Process " << my_rank << ": " << "print rowSendBuffer2" << endl;
            cout << "[";
            for (int i = 0; i < numColsReceived; i++) {
                cout << " " << rowSendBuffer[i];
            }
            cout << "]" << endl;
            #endif

            for (int col = 0; col < numColsReceived; col++) {
                local_arrays[nextOffset][numRowsReceived - 1][col] = rowReceiveBuffer[col];
            }

            // MPI_Send(rowSendBuffer, numColsReceived, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD);
        }

        #ifdef MPI_DEBUG_LOGGING
        cout << "Process " << my_rank << ": " << "print local_arrays[nextOffset] updated" << endl;
        for (int i = 0; i < numRowsReceived; i++) {
            cout << "[";
            for (int j = 0; j < numColsReceived; j++) {
                cout << local_arrays[nextOffset][i][j] << " ";
            }
            cout << "]" << endl;
        }
        #endif

        offset = nextOffset;
        nextOffset = (offset + 1) % (maxOffset + 1);

        int noUpdate = rowsNoUpdates == numRowsReceived - overlap;
        MPI_Allreduce(&noUpdate, &exit, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);

        // TODO: fix early stop check
        if (my_rank == 0 && exit) {
            #ifdef EARLY_STOP_LOGGING
            cout << "Iteration: " << currentIteration + 1 << ", rows without updates: " << atomicRowsNoUpdates << endl;
            #endif

            cout << "Exiting early on iteration: " << currentIteration + 1 << " because there was no update" << endl;

            #ifdef EARLY_STOP_LOGGING
            cout << arrayToString(local_arrays[offset],  rows, columns, border) << endl;
            #endif
        }
    }

    secondSendBuffer = new int[(numRowsReceived - overlap) * numColsReceived];

    // populate second send buffer
    for (int row = overlapBorder; row < numRowsReceived - overlapBorder; row++) {
        secondSendBuffer[((row - 1) * numColsReceived)] = 0;
        secondSendBuffer[((row - 1) * numColsReceived) + numColsReceived - border] = 0;

        for (int column = border; column < numColsReceived - border; column++) {
            secondSendBuffer[((row - 1) * numColsReceived) + column] = local_arrays[offset][row][column];
        }
    }

    #ifdef MPI_DEBUG_LOGGING
    cout << "Process " << my_rank << ": " << "print the secondSendBuffer" << endl;
    for (int i = 0; i < numRowsReceived - overlap; i++) {
        cout << "[";
        for (int j = 0; j < numColsReceived; j++) {
            cout << secondSendBuffer[(i * numColsReceived) + j] << " ";
        }
        cout << "]" << endl;
    }
    #endif

    // adjust sendCounts
    if (my_rank == 0) {
        int local_sum = 0;
        for (int i = 0; i < groups.size(); i++) {
            rowsToSend = groups.at(i).second - groups.at(i).first;

            sendCounts[i] = (rowsToSend * numColsReceived);
            displacements[i] = local_sum;
            local_sum += sendCounts[i];
        }

        rowsToSend = groups.at(0).second - groups.at(0).first;
        my_sendCount = sendCounts[0];

        secondReceiveBuffer = new int[(rows) * numColsReceived];
    } else {
        rowsToSend = groups.at(my_rank).second - groups.at(my_rank).first;
        my_sendCount = (rowsToSend * numColsReceived);
    }


    MPI_Gatherv(secondSendBuffer, my_sendCount, MPI_INT, secondReceiveBuffer, sendCounts, displacements, MPI_INT, 0,
                MPI_COMM_WORLD);


    if (my_rank == 0) {
        #ifdef MPI_DEBUG_LOGGING
        cout << "print the secondReceiveBuffer" << endl;
        for (int i = 0; i < rows; i++) {
            cout << "[";
            for (int j = 0; j < numColsReceived; j++) {
                cout << secondReceiveBuffer[(i * numColsReceived) + j] << " ";
            }
            cout << "]" << endl;
        }
        #endif

        for (int row = border; row < rows + border; row++) {
            for (int column = 0; column < numColsReceived; column++) {
                _arrays[0][row][column] = secondReceiveBuffer[((row - border) * numColsReceived) + column];
            }
        }

        #ifdef MPI_DEBUG_LOGGING
        cout << "Print array" << endl;
        cout << arrayToString(_arrays[0], rows, columns, border) << endl;
        #endif

        offset = 0;
    }

    // Finalize the MPI environment. No more MPI calls can be made after this
    if (my_rank == 0) {
        delete[] sendCounts;
        delete[] displacements;
        delete[] sendBuffer;
        delete[] secondReceiveBuffer;
    }
    delete[] receiveBuffer;
    delete[] secondSendBuffer;
    delete[] rowSendBuffer;
    delete[] rowReceiveBuffer;

    for (int i = 0; i < numArrays; i++) {
        if (local_arrays[i] != nullptr) {
            LibraryCode::deleteArray(local_arrays[i]);
        }
    }

    delete[] local_arrays;

    #endif
    // endregion


    // region STANDARD_CHECK_MPI_NONBLOCKING
    #ifdef STANDARD_CHECK_MPI_NONBLOCKING

    int overlapBorder = 1;
    int overlap = 2 * overlapBorder;

    int columnsToSend = columns + 2 * border;
    int rowsToSend;

    int my_recvCount = 0;
    int my_sendCount = 0;

    if (my_rank == 0) {
        sendCounts = new int[groups.size()];
        displacements = new int[groups.size()];

        int local_sum = 0;
        for (int i = 0; i < groups.size(); i++) {
            rowsToSend = groups.at(i).second - groups.at(i).first + overlap;

            sendCounts[i] = ((rowsToSend) * columnsToSend);
            displacements[i] = local_sum;
            local_sum += sendCounts[i] - (overlap * columnsToSend);
        }

        rowsToSend = groups.at(0).second - groups.at(0).first + overlap;
        my_recvCount = sendCounts[0];
    } else {
        rowsToSend = groups.at(my_rank).second - groups.at(my_rank).first + overlap;

        my_recvCount = (rowsToSend * columnsToSend);
        my_sendCount = my_recvCount;
    }

    if (my_rank == 0) {
        sendBuffer = new int[(rows + 2 * border) * (columns + 2 * border)];

        for (int row = 0; row < rows + 2 * border; row++) {
            for (int col = 0; col < columns + 2 * border; col++) {
                sendBuffer[row * (columns + 2 * border) + col] = _arrays[0][row][col];
            }
        }

        #ifdef MPI_DEBUG_LOGGING
        cout << "print the sendBuffer" << endl;
        for (int i = 0; i < rows + 2 * border; i++) {
            cout << "[";
            for (int j = 0; j < columns + 2 * border; j++) {
                cout << sendBuffer[i * (columns + 2 * border) + j] << " ";
            }
            cout << "]" << endl;
        }
        #endif
    }

    receiveBuffer = new int[my_recvCount];

    MPI_Scatterv(sendBuffer, sendCounts, displacements, MPI_INT, receiveBuffer, my_recvCount, MPI_INT, 0,
                 MPI_COMM_WORLD);


    int numRowsReceived = groups.at(my_rank).second - groups.at(my_rank).first + overlap;
    #ifdef MPI_DEBUG_LOGGING
    cout << "Process " << my_rank << ": " << "numRowsReceived = " << numRowsReceived << endl;
    cout << "Process " << my_rank << ": " << "group first: " << groups.at(my_rank).first << ", group second: " << groups
            .at(my_rank).second << endl;
    #endif

    int numColsReceived = columnsToSend;

    #ifdef MPI_DEBUG_LOGGING
    cout << "Process " << my_rank << ": " << "numColsReceived = " << numColsReceived << endl;
    #endif

    rowSendBuffer = new int[numColsReceived];
    rowReceiveBuffer = new int[numColsReceived];

    for (int i = 0; i < numColsReceived; i++) {
        rowSendBuffer[i] = 0;
        rowReceiveBuffer[i] = 0;
    }

    int ***local_arrays = new int **[numArrays];

    // special case for first array, because borders are sent through mpi
    local_arrays[0] = LibraryCode::allocateArray<int>(numRowsReceived, numColsReceived);

    for (int i = 1; i < numArrays; i++) {
        local_arrays[i] = LibraryCode::allocateArray<int>(numRowsReceived, numColsReceived);

        // Initialize the borders
        for (int row = 0; row < numRowsReceived; row++) {
            for (int colInset = 0; colInset < border; colInset++) {
                local_arrays[i][row][colInset] = 0;
                local_arrays[i][row][numColsReceived - colInset - 1] = 0;
            }
        }

        for (int col = 0; col < numColsReceived; col++) {
            for (int rowInset = 0; rowInset < border; rowInset++) {
                local_arrays[i][rowInset][col] = 0;
                local_arrays[i][numRowsReceived - rowInset - 1][col] = 0;
            }
        }
    }

    // copy received data to local array
    for (int row = 0; row < numRowsReceived; row++) {
        for (int col = 0; col < numColsReceived; col++) {
            local_arrays[0][row][col] = receiveBuffer[(row * numColsReceived) + col];
        }
    }

    #ifdef MPI_DEBUG_LOGGING
    cout << "Process " << my_rank << ": " << "print local_arrays[0]" << endl;
    for (int i = 0; i < numRowsReceived; i++) {
        cout << "[";
        for (int j = 0; j < numColsReceived; j++) {
            cout << local_arrays[0][i][j] << " ";
        }
        cout << "]" << endl;
    }
    #endif

    int exit = 0;


    for (int currentIteration = 0; currentIteration < iterations && !exit; currentIteration++) {
        rowsNoUpdates = 0;
        colsNoUpdates = 0;

        for (int row = overlapBorder; row < numRowsReceived - overlapBorder; row++) {
            for (int column = border; column < numColsReceived - border; column++) {
                int value = local_arrays[offset][row - 1][column - 1] + local_arrays[offset][row - 1][column] +
                            local_arrays[offset][row - 1][column + 1]
                            + local_arrays[offset][row][column - 1] + local_arrays[offset][row][column + 1]
                            + local_arrays[offset][row + 1][column - 1] + local_arrays[offset][row + 1][column] +
                            local_arrays[offset][row + 1][column + 1];

                int oldVal = local_arrays[offset][row][column];
                int newVal = (value == 3) ? 1 : (value == 2) ? oldVal : 0;

                local_arrays[nextOffset][row][column] = newVal;
                colsNoUpdates += (oldVal == newVal);

                // cout << "[" << row << ", " << column << ", s:" << sum << "] ";
            }
            rowsNoUpdates += (colsNoUpdates == columns);
            colsNoUpdates = 0;
            // cout << endl;
        }

        MPI_Request sendTopRow, sendBottomRow, recvTopRow, recvBottomRow;
        MPI_Status status;

        // MPI_Barrier(MPI_COMM_WORLD);

        // cout << "Process " << my_rank << ": " << "print nextoffset localArray" << endl;
        // cout << arrayToString(local_arrays[nextOffset], numRowsReceived, numColsReceived, 0) << endl;

        // MPI_Barrier(MPI_COMM_WORLD);

        // send Edges
        // send upper row
        if (my_rank != 0) {
            for (int col = 0; col < numColsReceived; col++) {
                rowSendBuffer[col] = local_arrays[nextOffset][1][col];
            }

            #ifdef MPI_DEBUG_LOGGING
            cout << "Process " << my_rank << ": " << "print rowSendBuffer" << endl;
            cout << "[";
            for (int i = 0; i < numColsReceived; i++) {
                cout << " " << rowSendBuffer[i];
            }
            cout << "]" << endl;
            #endif

            MPI_Isend(rowSendBuffer, numColsReceived, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, &sendTopRow);
        }

        // MPI_Barrier(MPI_COMM_WORLD);

        // send lower row
        if (my_rank != world_size - 1) {
            for (int col = 0; col < numColsReceived; col++) {
                rowSendBuffer[col] = local_arrays[nextOffset][numRowsReceived - 2][col];
            }

            #ifdef MPI_DEBUG_LOGGING
            cout << "Process " << my_rank << ": " << "print rowSendBuffer2" << endl;
            cout << "[";
            for (int i = 0; i < numColsReceived; i++) {
                cout << " " << rowSendBuffer[i];
            }
            cout << "]" << endl;
            #endif

            MPI_Isend(rowSendBuffer, numColsReceived, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, &sendBottomRow);
        }

        // MPI_Barrier(MPI_COMM_WORLD);


        // receive lower row
        if (my_rank != world_size - 1) {
            MPI_Irecv(rowReceiveBuffer, numColsReceived, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, &recvBottomRow);
        }

        // receive upper row
        if (my_rank != 0) {
            MPI_Irecv(rowReceiveBuffer, numColsReceived, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, &recvTopRow);
        }


        int noUpdate = rowsNoUpdates == numRowsReceived - overlap;
        MPI_Allreduce(&noUpdate, &exit, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);

        // TODO: fix early stop check
        if (exit) {
            #ifdef EARLY_STOP_LOGGING
            cout << "Iteration: " << currentIteration + 1 << ", rows without updates: " << atomicRowsNoUpdates << endl;
            #endif

            if (my_rank == 0) {
                cout << "Exiting early on iteration: " << currentIteration + 1 << " because there was no update" << endl;
            }

            MPI_Cancel(&sendBottomRow);
            MPI_Cancel(&sendTopRow);
            MPI_Cancel(&recvBottomRow);
            MPI_Cancel(&recvTopRow);

            MPI_Wait(&sendBottomRow, MPI_STATUS_IGNORE);
            MPI_Wait(&sendTopRow, MPI_STATUS_IGNORE);
            MPI_Wait(&recvBottomRow, MPI_STATUS_IGNORE);
            MPI_Wait(&recvTopRow, MPI_STATUS_IGNORE);

            offset = nextOffset;
            nextOffset = (offset + 1) % (maxOffset + 1);

            break;

            #ifdef EARLY_STOP_LOGGING
            cout << arrayToString(local_arrays[offset],  rows, columns, border) << endl;
            #endif
        }


        //TODO: this doesn't work because the send buffer is updated in the second send. Need to add another buffer for sending and receiving

        // receive lower row
        if (my_rank != world_size - 1) {
            MPI_Wait(&sendBottomRow, &status);
            MPI_Wait(&recvBottomRow, &status);

            #ifdef MPI_DEBUG_LOGGING
            cout << "Process " << my_rank << ": " << "print rowReceiveBuffer" << endl;
            cout << "[";
            for (int i = 0; i < numColsReceived; i++) {
                cout << " " << rowReceiveBuffer[i];
            }
            cout << "]" << endl;
            #endif

            for (int col = 0; col < numColsReceived; col++) {
                local_arrays[nextOffset][numRowsReceived - 1][col] = rowReceiveBuffer[col];
            }
        }

        // MPI_Barrier(MPI_COMM_WORLD);

        // receive upper row
        if (my_rank != 0) {
            MPI_Wait(&sendTopRow, &status);
            MPI_Wait(&sendTopRow, &status);

            #ifdef MPI_DEBUG_LOGGING
            cout << "Process " << my_rank << ": " << "print rowReceiveBuffer2" << endl;
            cout << "[";
            for (int i = 0; i < numColsReceived; i++) {
                cout << " " << rowReceiveBuffer[i];
            }
            cout << "]" << endl;
            #endif

            for (int col = 0; col < numColsReceived; col++) {
                local_arrays[nextOffset][0][col] = rowReceiveBuffer[col];
            }
        }
        // MPI_Barrier(MPI_COMM_WORLD);



        #ifdef MPI_DEBUG_LOGGING
        cout << "Process " << my_rank << ": " << "print local_arrays[nextOffset] updated" << endl;
        for (int i = 0; i < numRowsReceived; i++) {
            cout << "[";
            for (int j = 0; j < numColsReceived; j++) {
                cout << local_arrays[nextOffset][i][j] << " ";
            }
            cout << "]" << endl;
        }
        #endif


        offset = nextOffset;
        nextOffset = (offset + 1) % (maxOffset + 1);
    }

    secondSendBuffer = new int[(numRowsReceived - overlap) * numColsReceived];

    // populate second send buffer
    for (int row = overlapBorder; row < numRowsReceived - overlapBorder; row++) {
        secondSendBuffer[((row - 1) * numColsReceived)] = 0;
        secondSendBuffer[((row - 1) * numColsReceived) + numColsReceived - border] = 0;

        for (int column = border; column < numColsReceived - border; column++) {
            secondSendBuffer[((row - 1) * numColsReceived) + column] = local_arrays[offset][row][column];
        }
    }

    #ifdef MPI_DEBUG_LOGGING
    cout << "Process " << my_rank << ": " << "print the secondSendBuffer" << endl;
    for (int i = 0; i < numRowsReceived - overlap; i++) {
        cout << "[";
        for (int j = 0; j < numColsReceived; j++) {
            cout << secondSendBuffer[(i * numColsReceived) + j] << " ";
        }
        cout << "]" << endl;
    }
    #endif

    // adjust sendCounts
    if (my_rank == 0) {
        int local_sum = 0;
        for (int i = 0; i < groups.size(); i++) {
            rowsToSend = groups.at(i).second - groups.at(i).first;

            sendCounts[i] = (rowsToSend * numColsReceived);
            displacements[i] = local_sum;
            local_sum += sendCounts[i];
        }

        rowsToSend = groups.at(0).second - groups.at(0).first;
        my_sendCount = sendCounts[0];

        secondReceiveBuffer = new int[(rows) * numColsReceived];
    } else {
        rowsToSend = groups.at(my_rank).second - groups.at(my_rank).first;
        my_sendCount = (rowsToSend * numColsReceived);
    }


    MPI_Gatherv(secondSendBuffer, my_sendCount, MPI_INT, secondReceiveBuffer, sendCounts, displacements, MPI_INT, 0,
                MPI_COMM_WORLD);


    if (my_rank == 0) {
        #ifdef MPI_DEBUG_LOGGING
        cout << "print the secondReceiveBuffer" << endl;
        for (int i = 0; i < rows; i++) {
            cout << "[";
            for (int j = 0; j < numColsReceived; j++) {
                cout << secondReceiveBuffer[(i * numColsReceived) + j] << " ";
            }
            cout << "]" << endl;
        }
        #endif

        for (int row = border; row < rows + border; row++) {
            for (int column = 0; column < numColsReceived; column++) {
                _arrays[0][row][column] = secondReceiveBuffer[((row - border) * numColsReceived) + column];
            }
        }

        #ifdef MPI_DEBUG_LOGGING
        cout << "Print array" << endl;
        cout << arrayToString(_arrays[0], rows, columns, border) << endl;
        #endif

        offset = 0;
    }

    // Finalize the MPI environment. No more MPI calls can be made after this
    if (my_rank == 0) {
        delete[] sendCounts;
        delete[] displacements;
        delete[] sendBuffer;
        delete[] secondReceiveBuffer;
    }
    delete[] receiveBuffer;
    delete[] secondSendBuffer;
    delete[] rowSendBuffer;
    delete[] rowReceiveBuffer;

    for (int i = 0; i < numArrays; i++) {
        if (local_arrays[i] != nullptr) {
            LibraryCode::deleteArray(local_arrays[i]);
        }
    }

    delete[] local_arrays;


    #endif

    MPI_Finalize();

    if (my_rank == 0) {
        end = chrono::system_clock::now();

        long seconds = chrono::duration_cast<chrono::seconds>(end - start).count();
        long ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        long decimal = ms - seconds * 1000;
        cout << "System took: " << seconds << "." << decimal << " seconds to run" << endl;

        if (rows * columns < printThreshold * printThreshold) {
            cout << "end matrix " << endl << arrayToString(_arrays[offset], rows, columns, border);
        }

        if (useInitializerList) {
            if (iterations == 2) {
                test = test_secondIteration;
                cout << "using secondIteration test vector" << endl;
            }
            if (rows * columns < printThreshold * printThreshold) {
                bool success = true;

                for (int i = 0; i < rows; i++) {
                    for (int j = 0; j < columns; j++) {
                        cout << "[t:" << test.at((i * rows) + j) << ", r:" << (
                            _arrays[offset][i + border][j + border] == 1 ? 1 : 0) << "] ";
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
                             outputDirectory + fileName.str() + std::to_string(fileNum) + ".txt") != filesInDirectory.
                   end()) {
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


        const double percent = (sum / static_cast<double>(rows * columns)) * 100.0;
        cout << "percent: " << percent << "%" << endl;

        for (int i = 0; i < maxOffset; i++) {
            LibraryCode::deleteArray(_arrays[i]);
        }
        delete[] _arrays;
    }


    return 0;
}
