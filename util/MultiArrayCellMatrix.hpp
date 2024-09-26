//
// Created by motst on 9/26/2024.
//

#ifndef CS481_HPC_MULTIARRAYCELLMATRIX_HPP
#define CS481_HPC_MULTIARRAYCELLMATRIX_HPP

#include "CellMatrix.hpp"
#include "LibraryCode.hpp"

namespace util {

    class MultiArrayCellMatrix : ICellMatrix {
    public:

        MultiArrayCellMatrix(int rows, int columns, int numArrays, const int border = 1);

        ~MultiArrayCellMatrix() override;

        bool inline set(const int row, const int column, bool val, const int offset) override {
            _arrays[offset][row + 1][column + 1] = val;
            return false;
        }

        bool inline set(const int row, const int column, bool val) override {
            return set(row, column, val, this->getOffset());
        }

        [[nodiscard]] bool inline get(const int row, const int column, const int offset) const override {
            return _arrays[offset][row + 1][column + 1];
        }

        [[nodiscard]] bool inline get(const int row, const int column) const override{
            return get(row, column, this->getOffset());
        }

        [[nodiscard]] int inline getVerticalWindow(const int row, const int column, const int offset) const override {
            return get(row - 1, column, offset) +
                   get(row, column, offset) +
                   get(row + 1, column, offset);
        }

        [[nodiscard]] int inline getVerticalWindow(const int row, const int col) const override {
            return getVerticalWindow(row, col, this->getOffset());
        }

    private:
        uint8_t*** _arrays;
    };
} // util

#endif //CS481_HPC_MULTIARRAYCELLMATRIX_HPP
