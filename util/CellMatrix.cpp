//
// Created by motst on 9/5/2024.
//

#include "CellMatrix.hpp"

using namespace std;

namespace util {

    CellMatrix::CellMatrix(const int rows, const int columns, const int maxOffset)
            : ICellMatrix(rows, columns, maxOffset, 1)
    {
        _2DGrid = LibraryCode::allocateArray(rows, columns * (_maxOffset + 1));
    }

    CellMatrix::CellMatrix(const int size, const int maxOffset)
            : CellMatrix(size, size, maxOffset)
    {}

    CellMatrix::~CellMatrix() {
        LibraryCode::deleteArray(_2DGrid);
    }

    int CellMatrix::getLocation(const int row, const int column, const int offset) const {
        return ((row * _columns + column) * (_maxOffset + 1)) + offset;
    }

    int CellMatrix::getWord(const int row, const int column, const int offset) const {
        int location = ((row * _columns + column) * (_maxOffset + 1)) + offset;
        return location / 64;
    }

    int CellMatrix::getWord(const int location) {
        return location / 64;
    }

    int CellMatrix::getBit(const int row, const int column, const int offset) const {
        int location = ((row * _columns + column) * (_maxOffset + 1)) + offset;
        return location % 64;
    }

    int CellMatrix::getBit(const int location) {
        return location % 64;
    }

    bool CellMatrix::set(const int row, const int column, const bool val) {
        return set(row, column, val, getOffset());
    }

    bool CellMatrix::set(const int row, const int column, const bool val, const int offset) {
        if (offset > _maxOffset || offset < 0) {
            cout << "getReceived invalid offset: " << offset << endl;
        }

        // create an infinite border of zeroes around the grid.
        if (row < 0 || row > this->rows() || column < 0 || column > this->columns()) {
            #ifdef CELL_MATRIX_DEBUG_LOGGING
            cout << "Error: Attempting to set out of range element will do nothing." << endl;
            #endif

            return false;
        }

        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "inside main set row: " << row << " column: " << column << " val: " << val << " offset: " << offset << endl;
        #endif

        if (val) {

            bool oldVal = _2DGrid[row][(_columns * offset) + column];
            _2DGrid[row][(_columns * offset) + column] = 1;
            return oldVal != val;

        }
        else {
            bool oldVal = _2DGrid[row][(_columns * offset) + column];
            _2DGrid[row][(_columns * offset) + column] = 0;
            return oldVal != val;
        }
    }

    bool CellMatrix::integratedSet(const int row, const int column, std::function<bool(bool)> evaluator,
                                   const int previousOffset) {
        return integratedSet(row, column, std::move(evaluator), previousOffset, getOffset());
    }

    bool CellMatrix::integratedSet(const int row, const int column, std::function<bool(bool)> evaluator, const int previousOffset, const int offset) {
        if (offset > _maxOffset || offset < 0) {
            cout << "getReceived invalid offset: " << offset << endl;
        }

        // create an infinite border of zeroes around the grid.
        if (row < 0 || row > _columns || column < 0 || column > _rows) {
            #ifdef CELL_MATRIX_DEBUG_LOGGING
            cout << "Error: Attempting to set out of range element will do nothing." << endl;
            #endif

            return false;
        }


        bool val, oldVal;

        oldVal = _2DGrid[row][(_columns * previousOffset) + column];
//        oldVal = _2DGrid[row][(column * (previousOffset+ 1)) + previousOffset]; // TODO: fix if changes go through

        val = evaluator(oldVal);

        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "inside integrated set row: " << row << " column: " << column << " val: " << val << " offset: " << offset << endl;
        #endif

        if (val) {
            _2DGrid[row][(_columns * offset) + column] = 1;
//            _2DGrid[row][(column * (offset+ 1)) + offset] = 1;
        }
        else {
                _2DGrid[row][(_columns * offset) + column] = 0;
//                _2DGrid[row][(column * (offset+ 1)) + offset] = 0;
        }

        return oldVal != val;

    }

    [[nodiscard]] bool CellMatrix::get(const int row, const int column) const {
        return get(row, column, getOffset());
    }

    bool CellMatrix::get(const int row, const int column, const int offset) const {
        if (offset > _maxOffset || offset < 0) {
            cout << "getReceived invalid offset: " << offset << endl;
        }

        // create an infinite border of zeroes around the grid.
        if (row < 0 || row >= _columns || column < 0 || column >= _rows) {
            #ifdef CELL_MATRIX_DEBUG_LOGGING
            cout << "Debug: Accessing out of range cell returning zero." << endl;
            #endif

            return false;
        }

        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "in main get() row: " << row << " column: " << column << " offset: " << offset << endl;
        #endif


        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "raw byte: " << std::bitset<64>(_2DGrid[row][(_columns * offset) + column]) << endl;
        #endif

        bool val;

        val = _2DGrid[row][(_columns * offset) + column] == 1;

        return val;
    }

    [[nodiscard]] int CellMatrix::getVerticalWindow(const int row, const int column) const {
        return getVerticalWindow(row, column, getOffset());
    }

    [[nodiscard]] int CellMatrix::getVerticalWindow(const int row, const int column, const int offset) const {
        if (offset > _maxOffset || offset < 0) {
            cout << "getVerticalWindowReceived invalid offset: " << offset << endl;
        }

        // create an infinite border of zeroes around the grid.
        if (row < -1 || row > _rows || column < -1 || column > _columns) {
            #ifdef CELL_MATRIX_DEBUG_LOGGING
            cout << "Debug: Accessing out of range column for window returning zero." << endl;
            #endif

            return 0;
        }

        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "in main get() row: " << row << " column: " << column << " offset: " << offset << endl;
        #endif


        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "raw byte: " << std::bitset<64>(_2DGrid[row][(_columns * offset) + column]) << endl;
        #endif

        int val;

        val = _2DGrid[row][(_columns * offset) + column];

        // TODO: make this much more efficient by removing if statements

        val += row > 0 ? (_2DGrid[row - 1][(_columns * offset) + column]) : 0;
        val += row < (this->rows() - 1) ? (_2DGrid[row + 1][(_columns * offset) + column]) : 0;

//        val = _2DGrid[row][(column * (offset+ 1)) + offset] == 1;
//
//        val += row > 0 ? (_2DGrid[row - 1][(column * (offset+ 1)) + offset]) : 0;
//        val += row < (this->rows() - 1) ? (_2DGrid[row + 1][(column * (offset+ 1)) + offset]) : 0;


        return val;
    }
}
