//
// Created by motst on 9/26/2024.
//

#ifndef CS481_HPC_LIBRARYCODE_HPP
#define CS481_HPC_LIBRARYCODE_HPP

#include <memory>

namespace util::LibraryCode {
    uint8_t **allocateArray(int rows, int cols);

    void deleteArray(uint8_t** array);
}

#endif //CS481_HPC_LIBRARYCODE_HPP
