//
// Created by motst on 9/5/2024.
//

#ifndef CELL_MATRIX_HPP
#define CELL_MATRIX_HPP

#include <chrono>
#include <iostream>
#include <random>
#include <utility>
#include <vector>
#include <sstream>
#include <mutex>
#include <memory>
#include <bitset>
#include <functional>

//#define CELL_MATRIX_DEBUG_LOGGING

//#define USE_ARRAY
#define USE_ARRAY_2D
// #define USE_VECTOR

namespace util {
    // row major
    class CellMatrix {
    public:
        CellMatrix(int rows, int columns, int maxOffset = 1);

        explicit CellMatrix(int size, int maxOffset = 1);

        ~CellMatrix();

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

        [[nodiscard]] inline int rows() const {
            return _rows;
        }

        [[nodiscard]] inline int columns() const {
            return _columns;
        }

        bool set(int row, int column, bool val);

        bool set(int row, int column, bool val, int offset);

        bool integratedSet(int row, int column, std::function<bool(bool)> evaluator, int previousOffset);

        bool integratedSet(int row, int column, std::function<bool(bool)> evaluator, int previousOffset, int offset);

        [[nodiscard]] bool get(int row, int column) const;

        [[nodiscard]] bool get(int row, int column, int offset) const;

        [[nodiscard]] int getVerticalWindow(int row, int col) const;

        [[nodiscard]] int getVerticalWindow(int row, int column, int offset) const;

        [[nodiscard]] inline int getOffset() const {
            return _offset;
        }

        [[nodiscard]] inline int getNextOffset() const {
            return _nextOffset;
        }

        int incrementOffset(); // NOLINT(*-use-nodiscard)

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

        void allocArray(int rows, int cols, int maxOffset) {

//            _integerBlock = (uint8_t *)malloc(rows*cols*sizeof(uint8_t));
//            _2DGrid = (uint8_t **)malloc(rows*sizeof(uint8_t*));

            _integerBlock = new uint8_t[rows * cols * (maxOffset + 1)];
            _2DGrid = new uint8_t*[rows];

            for (int i = 0; i < rows; i++)
                _2DGrid[i] = &_integerBlock[i * cols * (maxOffset+ 1)];
        }

        void freeArray() {
            delete[] _integerBlock;
            delete[] _2DGrid;

//            delete[] arr[0];
//            delete[] arr;

//            free(&a[0][0]);
//            free(arr[0]);
//            free(arr);
        }

        #ifdef USE_VECTOR
        std::vector<uint64_t> _grid;
        #endif

        #ifdef USE_ARRAY
        uint64_t* _arrayGrid;
        #endif

        #ifdef USE_ARRAY_2D
        uint8_t* _integerBlock;
        uint8_t** _2DGrid;
        #endif

        int _rows, _columns;
        int _maxOffset = 1, _offset = 0, _nextOffset = 1;
    };
}

#endif //CELL_MATRIX_HPP