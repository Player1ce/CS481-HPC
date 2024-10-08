//
// Created by motst on 9/26/2024.
//

#ifndef CS481_HPC_LIBRARYCODE_HPP
#define CS481_HPC_LIBRARYCODE_HPP

#include <memory>

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
}

#endif //CS481_HPC_LIBRARYCODE_HPP
