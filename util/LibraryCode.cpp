//
// Created by motst on 9/26/2024.
//

#include "LibraryCode.hpp"

namespace util::LibraryCode {

    uint8_t** allocateArray(const int rows, const int cols) {
        uint8_t* block;
        uint8_t** grid;

        block = new uint8_t[rows * cols];
        grid = new uint8_t*[rows];

        for (int i = 0; i < rows; i++)
            grid[i] = &block[i * cols];

        return grid;
    }


    void deleteArray(uint8_t** array) {
        delete[] array[0];
        delete[] array;
    }
}