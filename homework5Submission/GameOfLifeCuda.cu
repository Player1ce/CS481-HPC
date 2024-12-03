//
// Created by motst on 10/3/2024.
//

#include "LibraryCode.hpp"
#include "FileIO.hpp"

#ifdef _OPENMP
# include <omp.h>
#endif

#include <cuda_runtime.h>

#include <stdio.h>
#include <random>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <thread>

using namespace std;
using namespace util;

#define CELL_UPDATE_DEBUG_LOGGING

template<typename T>
void fillFromVector(T **array, const int rows, const int columns, const vector<bool>& list, const int border) {
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

void checkCudaError(cudaError_t err) {
    if (err != cudaSuccess) {
        std::cerr << "CUDA Error: " << cudaGetErrorString(err) << std::endl;
        exit(EXIT_FAILURE);
    }
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

// TODO:  store cols no updates in shared memory
// TODO:  coalesce cols no updates efficiently using tree architecture?
// TODO:  store offset calculation in shared memory and only update in master thread
__global__ void standard_check_cuda(int *boards, const int board_rows, const int board_columns, int board_border,
                                    int offset, int nextOffset) {

    int allocation_rows = board_rows + 2 * board_border;
    int allocation_columns = board_columns + 2 * board_border;

    // printf("r: %d, c: %d : %d %c", (blockIdx.y * blockDim.y) + threadIdx.y , (blockIdx.x * blockDim.x) + threadIdx.x, boards[(threadIdx.y + board_border) * allocation_columns + threadIdx.x + 1],
    // '\n');

    // printf("blockx: %d, blocky: %d \n", blockIdx.x, blockIdx.y);
    int InnerCellsNoUpdate = 0;


    int calculatedRow = (blockIdx.y * blockDim.y) + threadIdx.y;
    int calculatedColumn = (blockIdx.x * blockDim.x) + threadIdx.x;


    int rawIndex = (calculatedRow + board_border) * allocation_columns +
                   /* account for vertical displacement */
                   board_border + /* border offset for the row won't get accounted for in vertical displacement */
                   calculatedColumn; /* account for thread horizontal displacement */


    int index = (offset * (allocation_rows * allocation_columns)) + /* account for offset */
                rawIndex;

    // printf("rawIndex: %d | index = %d | index limit: %d \n",
    //         rawIndex, index,
    //         allocation_rows * allocation_columns - (board_border + allocation_columns));


    // __syncthreads();

    int update = 0;
    if (calculatedRow < board_rows && calculatedColumn < board_columns) {
        // TODO: test this
        // int lowerRow = index + allocation_columns;
        // int upperRow = index - allocation_columns;

        int value = boards[index - allocation_columns - 1] + boards[index - allocation_columns] + boards[index - allocation_columns + 1]
                    + boards[index - 1] + boards[index + 1]
                    + boards[index + allocation_columns  - 1] + boards[index + allocation_columns] + boards[index + allocation_columns + 1];

        int oldVal = boards[index];
        int newVal = (value == 3) ? 1 : (value == 2) ? oldVal : 0;

        // set next cell
        boards[(nextOffset * (allocation_rows * allocation_columns)) + rawIndex] = newVal;

        // TODO: implement efficient update collection. Otherwise this will serialize
        update = oldVal == newVal;
    }


    // TODO: implement check for master thread
    // innerRowsNoUpdates += innerColsNoUpdates == columns;
    // innerColsNoUpdates = 0;
    //                cout << endl;
}

// theirs
//5000x1000: 46.3

// standalone checked
// 5000x1000: 25.7, 24.6


// standalone 10 thread checked
// 5000x1000: 11.4
// 5000x5000: 54, 57.185
// 10000x10000: 451.659


//#define EARLY_STOP_LOGGING


// #define STANDARD_CHECK
// #define STANDARD_CHECK_OMP
#define STANDARD_CHECK_CUDA
//#define STANDARD_CHECK_CUDA_NONBLOCKING

int main(int argc, char **argv) {
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

    // region CLI_arguments

    if (argc < 2) {
        cout << "Using coded constants" << endl;
    } else if (argc == 2) {
        cout << "Using Vector initializer" << endl;
        useInitializerList = true;
        rows = columns = 5;
        iterations = 1;
        numThreads = 1;
    } else if (argc == 3) {
        cout << "Using size: " << argv[1] << " and iterations: " << argv[2] << endl;
        rows = atoi(argv[1]);
        columns = rows;
        iterations = atoi(argv[2]);
    } else if (argc == 4) {
        cout << "Using size: " << argv[1] << " and iterations: " << argv[2] << " and numThreads: " << argv[3] << endl;
        rows = atoi(argv[1]);
        columns = rows;
        iterations = atoi(argv[2]);
        numThreads = atoi(argv[3]);
    } else if (argc == 5) {
        cout << "Using rows: " << argv[1] << " and iterations: " << argv[2] << " and numThreads: " << argv[3] <<
                " and filePath: " << argv[4] << std::endl;
        rows = atoi(argv[1]);
        columns = rows;
        iterations = atoi(argv[2]);
        numThreads = atoi(argv[3]);
        outputDirectory = argv[4];
        writeToFile = true;
    } else if (argc == 6) {
        cout << "Using rows: " << argv[1] << " and iterations: " << argv[2] << " and numThreads: " << argv[3] <<
                " and filePath: " << argv[4] << " and test file name:" << argv[5] << std::endl;
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
    // endregion CLI_arguments

    int printCount = max(iterations / 10, 1);

    int ***_arrays = new int **[2];
    int **_blocks = new int *[2];

    int allocation_rows = rows + (2 * border);
    int allocation_cols = columns + (2 * border);
    int allocation_size = allocation_rows * allocation_cols;

    cout << "allocation size: " << allocation_size << endl;

    // create the arrays and their borders. Also store the blocks for easy transfer to Cuda
    for (int i = 0; i < numArrays; i++) {
        int *block;
        int **grid;

        block = new int[allocation_rows * allocation_cols];
        grid = new int *[allocation_rows];

        for (int j = 0; j < allocation_rows; j++) {
            grid[j] = &block[j * allocation_cols];
        }

        _arrays[i] = grid;
        _blocks[i] = block;

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

    const int sum = getSum(_arrays[0], rows, columns, border);

    if (rows * columns < printThreshold * printThreshold) {
        cout << arrayToString(_arrays[0], rows, columns, border) << endl;
    }

    int colsNoUpdates = 0;
    int rowsNoUpdates = 0;

    bool updateOccurred = true;

    chrono::time_point<chrono::system_clock> start, end;

    // region cuda_initialization
    int *d_boards;
    checkCudaError(cudaMalloc(reinterpret_cast<void **>(&d_boards), sizeof(int) * allocation_size * 2));

    // get gpu properties
    int deviceId = 0; // Assuming you want to query the first GPU
    cudaDeviceProp deviceProp{};
    cudaGetDeviceProperties(&deviceProp, deviceId);

    int min_block_size = 128;
    int min_width = 32;
    int min_height = 4;

    // create dimensions for Cuda code:
    dim3 gridDimensions_2D;
    dim3 blockDimensions_2D;

    if (rows * columns < min_block_size) {
        blockDimensions_2D.x = rows;
        blockDimensions_2D.y = columns;
    } else {
        blockDimensions_2D.x = min_width;
        blockDimensions_2D.y = min_height;
    }

    if (rows < blockDimensions_2D.y) {
        blockDimensions_2D.y = rows;
    }

    // blockDimensions_2D.x = 1;
    // blockDimensions_2D.y = 1;

    gridDimensions_2D.x = std::ceil(static_cast<float>(rows) / static_cast<float>(blockDimensions_2D.x));
    gridDimensions_2D.y = std::ceil(static_cast<float>(columns) / static_cast<float>(blockDimensions_2D.y));

    int minGridSize;
    int blockSize;

    // cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize, standard_check_cuda, 0, 0);
    // cout << "minGridSize: " << minGridSize << endl;
    // cout << "blockSize: " << blockSize << endl;

    cout << "grid_x: " << gridDimensions_2D.x << endl;
    cout << "grid_y: " << gridDimensions_2D.y << endl;

    cout << "block_x: " << blockDimensions_2D.x << endl;
    cout << "block_y: " << blockDimensions_2D.y << endl;


    // endregion
    start = chrono::system_clock::now();


    // Copy first block from host to device
    checkCudaError(cudaMemcpy(d_boards, _blocks[0], sizeof(int) * allocation_size, cudaMemcpyHostToDevice));

    // Copy second block from host to device
    checkCudaError(cudaMemcpy(d_boards + allocation_size, _blocks[1], sizeof(int) * allocation_size,
                              cudaMemcpyHostToDevice));

    int currentIteration = 0;
    for (currentIteration = 0; currentIteration < iterations; currentIteration++) {
        // tested on 1000x1000 for 1000 iterations with 1 thread

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

        //region standard_check_cuda
#ifdef STANDARD_CHECK_CUDA

        standard_check_cuda<<<gridDimensions_2D,blockDimensions_2D>>>(d_boards, rows, columns, border, offset, nextOffset);

        checkCudaError(cudaDeviceSynchronize());

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

#ifdef STANDARD_CHECK
    // Copy current block from host to device
    checkCudaError(cudaMemcpy(d_boards + (allocation_size * offset), _blocks[offset], sizeof(int) * allocation_size,
                              cudaMemcpyHostToDevice));

#endif

    // test Memcpy. If using the test case this will cause the result to be wrong unless the board copies correctly.
    if (useInitializerList) {
        cout << "Editing element to make test fail if Memcpy doesn't work." << endl;
        _arrays[offset][3][3] = 1;
    }

    // Copy cuda data back to array
    checkCudaError(cudaMemcpy(_blocks[offset], d_boards + (allocation_size * offset),
                              sizeof(int) * allocation_size, cudaMemcpyDeviceToHost));


    if (rows * columns < printThreshold * printThreshold) {
        cout << "end matrix " << endl << arrayToString(_arrays[offset], rows, columns, border);
    }

    if (useInitializerList) {
        if (rows * columns < printThreshold * printThreshold) {
            if (iterations == 2) {
                test = test_secondIteration;
            }

            bool success = true;

            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < columns; j++) {
                    cout << "[t:" << test.at((i * rows) + j) << ", r:" << (_arrays[offset][i + border][j + border] == 1
                                                                               ? 1
                                                                               : 0) << "] ";
                    if (test.at((i * rows) + j) != (_arrays[offset][i + border][j + border] == 1 ? 1 : 0)) {
                        success = false;
                    }
                }
                std::cout << std::endl;
            }

            cout << "Success: " << boolalpha << success << endl;
        }
    }

    const double percent = (sum / static_cast<double>(rows * columns)) * 100.0;
    cout << "percent: " << percent << "%" << endl;

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
        cout << boolalpha << "Test file was the same: " << success << endl;
    }

    for (int i = 0; i < maxOffset; i++) {
        LibraryCode::deleteArray(_arrays[i]);

        delete[] _arrays;
    }

    // free cuda memory
    checkCudaError(cudaFree(d_boards));


    return 0;
}
