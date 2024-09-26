//
// Created by motst on 9/5/2024.
//

#ifndef CELL_MATRIX_HPP
#define CELL_MATRIX_HPP

#include <iostream>
#include <sstream>
#include <vector>
#include <random>
#include <memory>
#include <bitset>
#include <functional>

#include "LibraryCode.hpp"
#include "ICellMatrix.hpp"

//#define CELL_MATRIX_DEBUG_LOGGING

//TODO: make a matrix that uses two arrays to avoid math operations and see how that goes

namespace util {
    // row major
    class CellMatrix : public ICellMatrix {
    public:
        CellMatrix(int rows, int columns, int maxOffset = 1);

        explicit CellMatrix(int size, int maxOffset = 1);

        ~CellMatrix() override;

        bool set(int row, int column, bool val) override;

        bool set(int row, int column, bool val, int offset) override;

        bool integratedSet(int row, int column, std::function<bool(bool)> evaluator, int previousOffset);

        bool integratedSet(int row, int column, std::function<bool(bool)> evaluator, int previousOffset, int offset);

        [[nodiscard]] bool get(int row, int column) const override;

        [[nodiscard]] bool get(int row, int column, int offset) const override;

        [[nodiscard]] int getVerticalWindow(int row, int column) const override;

        [[nodiscard]] int getVerticalWindow(int row, int column, int offset) const override;

    private:
        uint8_t** _2DGrid;
    };
}

#endif //CELL_MATRIX_HPP