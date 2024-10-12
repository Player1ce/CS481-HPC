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
