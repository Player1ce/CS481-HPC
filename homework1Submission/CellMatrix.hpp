//
// Created by motst on 9/5/2024.
//

#ifndef CELL_MATRIX_HPP
#define CELL_MATRIX_HPP

#include <chrono>
#include <iostream>
#include <random>
#include <vector>
#include <sstream>
#include <mutex>
#include <memory>
#include <bitset>

//#define CELL_MATRIX_DEBUG_LOGGING

//#define USE_ARRAY
#define USE_ARRAY_2D
// #define USE_VECTOR

namespace util {
    // row major
    class CellMatrix {
    public:
        CellMatrix(const int rows, const int columns, const int maxOffset = 1)
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
            _2DGrid = new uint8_t*[rows]();
            for (int i = 0; i < rows; i++) {
                _2DGrid[i] = new uint8_t [columns * (_maxOffset + 1)];
            }
            #endif
        }

        explicit CellMatrix(const int size, const int maxOffset = 1)
            : CellMatrix(size, size, maxOffset)
        {}

        ~CellMatrix() {
            #ifdef USE_ARRAY
            delete[] _arrayGrid;
            _arrayGrid = nullptr;
            #endif

            #ifdef USE_ARRAY_2D
            delete[] _2DGrid;
            _2DGrid = nullptr;
            #endif
        }

        void fillWithRandom(int min= 0, int max = 1);

        void fillFromVector(const std::vector<bool>& list);

        [[nodiscard]] inline int getLocation(int row, int column, int offset) const;

        [[nodiscard]] inline int getWord(int row, int column, int offset) const;

        [[nodiscard]] inline int getWord(int location) const;

        [[nodiscard]] inline int getBit(int row, int column, int offset) const;

        [[nodiscard]] inline int getBit(int location) const;

        [[nodiscard]] inline bool isSquare() const {
            return _rows == _columns;
        }

        [[nodiscard]] int rows() const {
            return _rows;
        }

        [[nodiscard]] int columns() const {
            return _columns;
        }

        bool set(const int row, const int column, const bool val) {
            return set(row, column, val, _offset);
        }

        bool set(int row, int column, bool val, int offset);

        [[nodiscard]] bool get(const int row, const int column) const {
            return get(row, column, _offset);
        }

        [[nodiscard]] bool get(int row, int column, int offset) const;

        [[nodiscard]] int getOffset() const {
            return _offset;
        }

        [[nodiscard]] inline int getNextOffset() const {
            return _nextOffset;
        }

        int incrementOffset() { // NOLINT(*-use-nodiscard)
            _offset = getNextOffset();
            _nextOffset = calculateNextOffset();
            return _offset;
        }

        [[nodiscard]] int getSum() const;

        [[nodiscard]] std::string toString() const;

        friend std::ostream& operator<<(std::ostream& os, const CellMatrix& obj) {
            os << obj.toString();
            return os;
        }

    private:

        [[nodiscard]] inline int calculateNextOffset() const {
            return (_offset + 1) % (_maxOffset + 1);
        }

        #ifdef USE_VECTOR
        std::vector<uint64_t> _grid;
        #endif

        #ifdef USE_ARRAY
        uint64_t* _arrayGrid;
        #endif

        #ifdef USE_ARRAY_2D
        uint8_t** _2DGrid;
        #endif

        int _rows, _columns;
        int _maxOffset = 1, _offset = 0, _nextOffset = 1;
    };
}

#endif //CELL_MATRIX_HPP