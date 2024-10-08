//
// Created by motst on 9/26/2024.
//

#include "MultiArrayCellMatrix.hpp"

namespace util {

    MultiArrayCellMatrix::MultiArrayCellMatrix(const int rows, const int columns, const int numArrays, const int border)
            : ICellMatrix(rows, columns, numArrays, 1)
    {
        _arrays = new uint8_t**[numArrays+1];
        for (int i = 0; i < numArrays + 1; i++) {
            _arrays[i] = LibraryCode::allocateArray<uint8_t>(rows + (2 * border), columns + (2 * border));

            // Create teh border
            for (int row = 0; row < rows; row++) {
                for (int colInset = 0; colInset < border; colInset++) {
                    this->MultiArrayCellMatrix::set(row, 0 + colInset, false, i);
                    this->MultiArrayCellMatrix::set(row, columns - colInset, false, i);
                }
            }

            for (int col = 0; col < columns; col++) {
                for (int rowInset = 0; rowInset < border; rowInset++) {
                    this->MultiArrayCellMatrix::set(0 + rowInset, col, false, i);
                    this->MultiArrayCellMatrix::set(rows - rowInset, col, false, i);
                }
            }

        }
    }

    MultiArrayCellMatrix::~MultiArrayCellMatrix() {
        for (int i = 0; i < _maxOffset; i++) {
            LibraryCode::deleteArray(_arrays[i]);

            delete[] _arrays;
        }
    }
} // util