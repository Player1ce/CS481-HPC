//
// Created by motst on 9/26/2024.
//

#ifndef CS481_HPC_LIBRARYCODE_HPP
#define CS481_HPC_LIBRARYCODE_HPP

#include <memory>
#include <vector>

namespace util::LibraryCode {
    template <typename T>
    T** allocateArray(const int rows, const int cols) {
        T* block;
        T** grid;

        block = new T[rows * cols];
        grid = new T*[rows];

        for (int i = 0; i < rows; i++)
            grid[i] = &block[i * cols];

        return grid;
    }

    template <typename T>
    void deleteArray(T** array) {
        delete[] array[0];
        delete[] array;
    }


    /// Convert a 2D array to a 1D array and store the result in the given 1D array
    /// \note if you wish to add border to the new array, increase the rows or columns over the size of the original array.
    /// \tparam T The type for the Array
    /// \param inputArray The array to convert
    /// \param outputArray The array to store the data in
    /// \param rows The number of rows in the original array
    /// \param cols The number of columns in the original array
    /// \param borderToRemove If desired, the border to remove from the input array
    /// \param borderToAdd If desired, the border to add to the output array
    template<typename T>
    void convert2Dto1DArray_inPlace(T** inputArray, T* outputArray, const int rows, const int cols, const int borderToRemove = 0, const int borderToAdd = 0) {
        int borderDifference = borderToAdd - borderToRemove;
        int outputCols = cols + (2 * borderDifference);

        for (int inputRow = borderToRemove; inputRow < rows - borderToRemove; inputRow++) {
            for (int inputCol = borderToRemove; inputCol < cols - borderToRemove; inputCol++) {
                int outputRow = inputRow + borderDifference;
                int outputCol = inputCol + borderDifference;

                outputArray[(outputRow * outputCols) + outputCol] = inputArray[inputRow][inputCol];
            }
        }
    }

    /// Convert a 2D array to a 1D array and return the new 1D array
    /// \tparam T The type for the Array
    /// \param inputArray The array to convert
    /// \param rows The number of rows in the original array
    /// \param cols The number of columns in the original array
    /// \param borderToRemove If desired, the border to remove from the input array
    /// \param borderToAdd If desired, the border to add to the output array
    /// \return A new 1D array containing the data originally stored in the 2D array minus any border selected
    template<typename T>
    T* convert2Dto1DArray(T** inputArray, const int rows, const int cols, const int borderToRemove = 0, const int borderToAdd = 0) {
        int borderDifference = borderToAdd - borderToRemove;

        int outputRows = rows + 2*borderDifference;
        int outputCols = cols + 2*borderDifference;

        T* outputArray = new T[outputRows * outputCols];

        convert2Dto1DArray_inPlace<T>(inputArray, outputArray, rows, cols, borderToRemove, borderToAdd);

        return outputArray;
    }

    /// Convert a 1D array to a 2D array and store the result in the given 2D array
    /// \note if you wish to add border to the new array, increase the rows or columns over the size of the original array.
    /// \tparam T The type for the Array
    /// \param inputArray The array to convert
    /// \param outputArray The array to store the data in
    /// \param rows The number of rows in the original array
    /// \param cols The number of columns in the original array
    /// \param borderToRemove If desired, the border to remove from the input array
    template<typename T>
    void convert1Dto2DArray_inPlace(T* inputArray, T** outputArray, const int rows, const int cols, const int borderToRemove = 0, const int borderToAdd = 0) {
        int borderDifference = borderToAdd - borderToRemove;

        int outputCols = cols + 2 * borderDifference;

        for (int inputRow = borderToRemove; inputRow < rows - borderToRemove; inputRow++) {
            int outputRow = inputRow + borderDifference;

            outputArray[outputRow] = &inputArray[inputRow * outputCols];
        }
    }

    /// Convert a 1D array to a 2D array and store the result in a new 2D array
    /// \note if you wish to add border to the new array, increase the rows or columns over the size of the original array.
    /// \tparam T The type for the Array
    /// \param inputArray The array to convert
    /// \param rows The number of rows in the original array
    /// \param cols The number of columns in the original array
    /// \param borderToRemove If desired, the border to remove from the input array
    /// \return A new 2D array containing the data originally stored in the 1D array minus any border selected
    template<typename T>
    T** convert1Dto2DArray(T* inputArray, const int rows, const int cols, const int borderToRemove = 0, const int borderToAdd = 0) {
        int borderDifference = borderToAdd - borderToRemove;

        int outputRows = rows + (2 * borderDifference);
        int outputCols = cols + (2 * borderDifference);

        auto outputArray = allocateArray<T>(outputRows, outputCols);

        convert1Dto2DArray_inPlace<T>(inputArray, outputArray, rows, cols, borderToRemove, borderToAdd);

        return outputArray;
    }

    std::vector<std::pair<int, int>> calculateRowGroups(const int rows, int numGroups) {

        if (numGroups < 0) {
            numGroups = 1;
        }

        if (numGroups > rows) {
            numGroups = rows;
        }

        int groupSize = rows/numGroups;
        int overhang = rows%numGroups;
        int previousOverhang = 0;
        int allocatedOverhang = 0;

        std::vector<std::pair<int, int>> rowGroups(numGroups);

        for (int i = 0; i < numGroups; i++) {
            rowGroups.at(i) = std::make_pair(i*groupSize + previousOverhang, (i+1)*groupSize + allocatedOverhang);

            if (i < overhang) {
                rowGroups.at(i).second += 1;
                previousOverhang = 1;
                allocatedOverhang++;
            } else {
                previousOverhang = 0;
            }
        }

        return rowGroups;
    }
}

#endif //CS481_HPC_LIBRARYCODE_HPP
