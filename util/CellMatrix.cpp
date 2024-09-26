//
// Created by motst on 9/5/2024.
//

#include "CellMatrix.hpp"

using namespace std;

namespace util {

    CellMatrix::CellMatrix(const int rows, const int columns, const int maxOffset)
            : _rows(rows),
              _columns(columns),
              _maxOffset(maxOffset),
              _offset(0),
              _nextOffset(calculateNextOffset())
    {
        #ifdef USE_VECTOR
        _grid.resize(((rows * columns + 63) * (_maxOffset + 1)) / 64);
        #endif

        #ifdef USE_ARRAY
        int arraySize = (((rows * columns + 63) * (_maxOffset + 1)) / 64);
            _arrayGrid = new uint64_t[arraySize]();
        #endif

        #ifdef USE_ARRAY_2D
//        _2DGrid = new uint8_t*[rows]();
//        for (int i = 0; i < rows; i++) {
//            _2DGrid[i] = new uint8_t [columns * (_maxOffset + 1)];
//        }
        allocArray(rows, columns, _maxOffset);
        #endif
    }

    CellMatrix::CellMatrix(const int size, const int maxOffset)
            : CellMatrix(size, size, maxOffset)
    {}

    CellMatrix::~CellMatrix() {
        #ifdef USE_ARRAY
        delete[] _arrayGrid;
            _arrayGrid = nullptr;
        #endif

        #ifdef USE_ARRAY_2D
//        delete[] _2DGrid;
//        _2DGrid = nullptr;

        freeArray();
        #endif
    }

    void CellMatrix::fillWithRandom(const int min, const int max)
    {
        // Create a random number generator
        std::random_device seed;
        std::mt19937 generator(seed());

        // Create a distribution for your desired range
        std::uniform_int_distribution<int> distribution(min, max);

        #ifdef CELL_MATRIX_DEGUB_LOGGING
            cout << "RNG created, filling matrix with numbers." << endl;
        #endif

        for (int i = 0; i < this->rows(); i++) {
            for (int j = 0; j < this->columns(); j++)
                this->set(i, j, distribution(generator));
        }
    }

    void CellMatrix::fillFromVector(const vector<bool> & list) {
        for (int i = 0; i < this->rows(); i++) {
            for (int j = 0; j < this->columns(); j++)
                this->set(i, j, list.at(this->columns() * i + j));
        }
    }

    int CellMatrix::getLocation(const int row, const int column, const int offset) const {
        return ((row * _columns + column) * (_maxOffset + 1)) + offset;
    }

    int CellMatrix::getWord(const int row, const int column, const int offset) const {
        int location = ((row * _columns + column) * (_maxOffset + 1)) + offset;
        return location / 64;
    }

    int CellMatrix::getWord(const int location) const {
        return location / 64;
    }

    int CellMatrix::getBit(const int row, const int column, const int offset) const {
        int location = ((row * _columns + column) * (_maxOffset + 1)) + offset;
        return location % 64;
    }

    int CellMatrix::getBit(const int location) const {
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

        #if defined(USE_ARRAY) || defined(USE_VECTOR)
        int location = getLocation(row, column, offset);
        int index = getWord(location);
        int bit = getBit(location);
        #endif

        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "data in main set location: " << location << " index: " << index << " bit: " << bit << endl;
        #endif

        if (val) {
            #ifdef USE_VECTOR
            bool oldVal = (_grid.at(index) & (static_cast<uint64_t>(1) << bit)) >= 1;
            _grid.at(index) |= (static_cast<uint64_t>(1) << bit);
            return oldVal != val;
            #endif

            #ifdef USE_ARRAY
            bool oldVal = (_arrayGrid[index] & (static_cast<uint64_t>(1) << bit)) >= 1;
            _arrayGrid[index] |= (static_cast<uint64_t>(1) << bit);
            return oldVal != val;
            #endif

            #ifdef USE_ARRAY_2D
            bool oldVal = _2DGrid[row][(_columns * offset) + column];
            _2DGrid[row][(_columns * offset) + column] = 1;
            return oldVal != val;
            #endif

            #ifdef CELL_MATRIX_DEBUG_LOGGING

            #ifdef USE_VECTOR
            cout << "raw byte: " << std::bitset<64>(_grid.at(index)) << endl;
            #endif

            #ifdef USE_ARRAY
            cout << "raw byte: " << std::bitset<64>(_arrayGrid[index]) << endl;
            #endif

            #endif
        }
        else {
            #ifdef USE_VECTOR
            bool oldVal = (_grid.at(index) & (static_cast<uint64_t>(1) << bit)) >= 1;
            _grid.at(index) &= ~(static_cast<uint64_t>(1) << bit);
            return val != oldVal;
            #endif

            #ifdef USE_ARRAY
            bool oldVal = (_arrayGrid[index] & (static_cast<uint64_t>(1) << bit)) >= 1;
            _arrayGrid[index] &= ~(static_cast<uint64_t>(1) << bit);
            return val != oldVal;
            #endif

            #ifdef USE_ARRAY_2D
            bool oldVal = _2DGrid[row][(_columns * offset) + column];
            _2DGrid[row][(_columns * offset) + column] = 0;
            return oldVal != val;
            #endif
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

        #if defined(USE_ARRAY) || defined(USE_VECTOR)
        int location = getLocation(row, column, offset);
        int index = getWord(location);
        int bit = getBit(location);

        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "data in main set location: " << location << " index: " << index << " bit: " << bit << endl;
        #endif

        #endif


        bool val, oldVal;

        // TODO: Update oldVal to use previous offset in all cases
        #ifdef USE_VECTOR
        oldVal = (_grid.at(index) & (static_cast<uint64_t>(1) << bit)) >= 1;
        #endif

        #ifdef USE_ARRAY
        oldVal = (_arrayGrid[index] & (static_cast<uint64_t>(1) << bit)) >= 1;
        #endif

        #ifdef USE_ARRAY_2D
        oldVal = _2DGrid[row][(_columns * previousOffset) + column];
//        oldVal = _2DGrid[row][(column * (previousOffset+ 1)) + previousOffset]; // TODO: fix if changes go through
        #endif

        val = evaluator(oldVal);

        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "inside integrated set row: " << row << " column: " << column << " val: " << val << " offset: " << offset << endl;
        #endif

        if (val) {
            #ifdef USE_VECTOR
            _grid.at(index) |= (static_cast<uint64_t>(1) << bit);
            #endif

            #ifdef USE_ARRAY
            _arrayGrid[index] |= (static_cast<uint64_t>(1) << bit);
            #endif

            #ifdef USE_ARRAY_2D
            _2DGrid[row][(_columns * offset) + column] = 1;
//            _2DGrid[row][(column * (offset+ 1)) + offset] = 1;
            #endif
        }
        else {
            #ifdef USE_VECTOR
            _grid.at(index) &= ~(static_cast<uint64_t>(1) << bit);
            #endif

            #ifdef USE_ARRAY
            _arrayGrid[index] &= ~(static_cast<uint64_t>(1) << bit);
            #endif

            #ifdef USE_ARRAY_2D
                _2DGrid[row][(_columns * offset) + column] = 0;
//                _2DGrid[row][(column * (offset+ 1)) + offset] = 0;
            #endif
        }



        #ifdef CELL_MATRIX_DEBUG_LOGGING

            #ifdef USE_VECTOR
            cout << "raw byte: " << std::bitset<64>(_grid.at(index)) << endl;
            #endif

            #ifdef USE_ARRAY
            cout << "raw byte: " << std::bitset<64>(_arrayGrid[index]) << endl;
            #endif

        #endif

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

        #if defined(USE_ARRAY) || defined(USE_VECTOR)
        int location = getLocation(row, column, offset);
        int index = getWord(location);
        int bit = getBit(location);

        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "data in main get - location: " << location << " index: " << index << " bit: " << bit << endl;
        #endif

        #endif


        #ifdef CELL_MATRIX_DEBUG_LOGGING
            
            #ifdef USE_VECTOR
            cout << "raw byte: " << std::bitset<64>(_grid.at(index)) << endl;
            cout << "raw byte after processing: " << std::bitset<64>((_grid.at(index) & (static_cast<uint64_t>(1) << bit))) << endl;
            cout << "value after processing: " << (_grid.at(index) & (static_cast<uint64_t>(1) << bit)) << endl;
            #endif

            #ifdef USE_ARRAY
            cout << "raw byte: " << std::bitset<64>(_arrayGrid[index]) << endl;
            cout << "raw byte after processing: " << std::bitset<64>((_arrayGrid[index] & (static_cast<uint64_t>(1) << bit))) << endl;
            cout << "value after processing: " << ((_arrayGrid[index]) & (static_cast<uint64_t>(1) << bit)) << endl;
            #endif

            #ifdef USE_ARRAY_2D
            cout << "raw byte: " << std::bitset<64>(_2DGrid[row][(_columns * offset) + column]) << endl;
            #endif

        #endif

        bool val;

        #ifdef USE_VECTOR
        val = (_grid.at(index) & (static_cast<uint64_t>(1) << bit)) >= 1;
        #endif

        #ifdef USE_ARRAY
        val = (_arrayGrid[index] & (static_cast<uint64_t>(1) << bit)) >= 1;
        #endif

        #ifdef USE_ARRAY_2D
        val = _2DGrid[row][(_columns * offset) + column] == 1;
        #endif

        return val;
    }

    [[nodiscard]] int CellMatrix::getVerticalWindow(const int row, const int col) const {
        return getVerticalWindow(row, col, getOffset());
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

        #if defined(USE_ARRAY) || defined(USE_VECTOR)
        int location = getLocation(row, column, offset);
        int index = getWord(location);
        int bit = getBit(location);

        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "data in main get - location: " << location << " index: " << index << " bit: " << bit << endl;
        #endif

        #endif


        #ifdef CELL_MATRIX_DEBUG_LOGGING

        #ifdef USE_VECTOR
            cout << "raw byte: " << std::bitset<64>(_grid.at(index)) << endl;
            cout << "raw byte after processing: " << std::bitset<64>((_grid.at(index) & (static_cast<uint64_t>(1) << bit))) << endl;
            cout << "value after processing: " << (_grid.at(index) & (static_cast<uint64_t>(1) << bit)) << endl;
            #endif

            #ifdef USE_ARRAY
            cout << "raw byte: " << std::bitset<64>(_arrayGrid[index]) << endl;
            cout << "raw byte after processing: " << std::bitset<64>((_arrayGrid[index] & (static_cast<uint64_t>(1) << bit))) << endl;
            cout << "value after processing: " << ((_arrayGrid[index]) & (static_cast<uint64_t>(1) << bit)) << endl;
            #endif

            #ifdef USE_ARRAY_2D
            cout << "raw byte: " << std::bitset<64>(_2DGrid[row][(_columns * offset) + column]) << endl;
            #endif

        #endif

        int val;

        #ifdef USE_VECTOR
        val = (_grid.at(index) & (static_cast<uint64_t>(1) << bit)) >= 1;

        val += index >= this->columns() ? ((_grid.at(index - this->columns()) & (static_cast<uint64_t>(1) << bit)) >= 1) : 0;
        val += index < this->rows() * (this->_columns() - 1) - 1 ? ((_grid.at(index - this->columns()) & (static_cast<uint64_t>(1) << bit)) >= 1) : 0;
        #endif

        #ifdef USE_ARRAY
        val = ((_arrayGrid[index] & (static_cast<uint64_t>(1) << bit)) >= 1);

        val += index >= this->columns() ? ((_arrayGrid[index - this->columns()] & (static_cast<uint64_t>(1) << bit)) >= 1) : 0;
        val += index < this->rows() * (this->_columns() - 1) - 1 ? ((_arrayGrid[index - this->columns()] & (static_cast<uint64_t>(1) << bit)) >= 1) : 0;
        #endif

        #ifdef USE_ARRAY_2D
        val = _2DGrid[row][(_columns * offset) + column];

        // TODO: make this much more efficient by removing if statements

        val += row > 0 ? (_2DGrid[row - 1][(_columns * offset) + column]) : 0;
        val += row < (this->rows() - 1) ? (_2DGrid[row + 1][(_columns * offset) + column]) : 0;

//        val = _2DGrid[row][(column * (offset+ 1)) + offset] == 1;
//
//        val += row > 0 ? (_2DGrid[row - 1][(column * (offset+ 1)) + offset]) : 0;
//        val += row < (this->rows() - 1) ? (_2DGrid[row + 1][(column * (offset+ 1)) + offset]) : 0;
        #endif

        return val;
    }

    int CellMatrix::incrementOffset() {
        _offset = getNextOffset();
        _nextOffset = calculateNextOffset();
        return _offset;
    }

    int CellMatrix::getSum() const {
        int sum = 0;
        for (int i = 0; i < this->rows(); i++) {
            for (int j = 0; j < this->columns(); j++) {
                sum += this->get(i, j);
            }
        }
        return sum;
    }

    std::string CellMatrix::toString() const {
        std::stringstream msg("");
        msg << "[" << endl;
        for (int i = 0; i < this->rows(); i++) {
            msg << "[ ";
            for (int j = 0; j < this->columns(); j++) {
                msg << this->get(i, j) << " ";
            }
            msg << "]" << endl;
        }
        msg << "]" << endl;
        return msg.str();
    }

}