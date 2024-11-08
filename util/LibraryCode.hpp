//
// Created by motst on 9/26/2024.
//

#ifndef CS481_HPC_LIBRARYCODE_HPP
#define CS481_HPC_LIBRARYCODE_HPP

#include <memory>
#include <vector>
#include <iostream>

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

    std::vector<std::pair<int, int>> calculateRowGroups(const int rows, int numGroups) {

        if (numGroups < 0) {
            numGroups = 1;
        }

        if (numGroups > rows) {
            numGroups = rows;
        }

        int groupSize = rows/numGroups;
        int overhang = rows%numGroups;
        int allocatedOverhang = 0;

        std::vector<std::pair<int, int>> rowGroups(numGroups);

        for (int i = 0; i < numGroups; i++) {
            int firstElement = i*groupSize + allocatedOverhang;
            rowGroups.at(i) = std::make_pair(firstElement, firstElement + groupSize);

            if (i < overhang) {
                rowGroups.at(i).second += 1;
                allocatedOverhang++;
            }
        }

        return rowGroups;
    }
}

#endif //CS481_HPC_LIBRARYCODE_HPP
