//
// Created by motst on 9/26/2024.
//

#ifndef CS481_HPC_MULTIARRAYCELLMATRIX_HPP
#define CS481_HPC_MULTIARRAYCELLMATRIX_HPP

#include "ICellMatrix.hpp"
#include "LibraryCode.hpp"

namespace util {

    class MultiArrayCellMatrix : public ICellMatrix {
    public:

        MultiArrayCellMatrix(int rows, int columns, int numArrays, int border = 1);

        ~MultiArrayCellMatrix() override;

        void inline set(const int row, const int column, const uint8_t val, const int offset) override {
            _arrays[offset][row + 1][column + 1] = val;
        }

        void inline set(const int row, const int column, const uint8_t val) override {
            set(row, column, val, this->getOffset());
        }

        void inline add(const int row, const int column, const uint8_t val, const int offset) override {
            _arrays[offset][row + 1][column + 1] += val;
        }

        void inline add(const int row, const int column, const uint8_t val) override {
            set(row, column, val, this->getOffset());
        }

        bool inline set_withCheck(const int row, const int column, const bool val,
                                  const int setOffset, const int testOffset) override
        {
            uint8_t oldVal = _arrays[testOffset][row + 1][column + 1];
            _arrays[setOffset][row + 1][column + 1] = val;
            return oldVal != val;
        }

        bool inline set_withCheck(const int row, const int column, const bool val) override {
            return set_withCheck(row, column, val, this->getOffset(), this->getOffset());
        }

        [[nodiscard]] bool inline get(const int row, const int column, const int offset) const override {
            return _arrays[offset][row + 1][column + 1];
        }

        [[nodiscard]] bool inline get(const int row, const int column) const override {
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
