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

namespace util {
    // row major
    class CellMatrix {
    public:
        CellMatrix(const int rows, const int columns, const int maxOffset = 1)
            : _rows(rows),
              _columns(columns),
              _maxOffset(maxOffset),
              _offset(0)
        {
            _grid.resize(((rows * columns + 63) * (_maxOffset * 1)) / 64);
        }

        explicit CellMatrix(const int size, const int maxOffset = 1)
            : _rows(size),
              _columns(size),
              _maxOffset(maxOffset),
              _offset(0)

        {
            _grid.resize(((size * size + 63) * (_maxOffset + 1)) / 64);
        }

        void fillWithRandom(int min= 0, int max = 1);

        void fillFromVector(const std::vector<bool>& list);

        bool isSquare() const {
            return _rows == _columns;
        }

        int rows() const {
            return _rows;
        }

        int columns() const {
            return _columns;
        }

        void set(const int row, const int column, const bool val) {
            set(row, column, val, _offset);
        }

        void set(int row, int column, bool val, int offset);


        bool get(const int row, const int column) const {
            return get(row, column, _offset);
        }

        bool get(int row, int column, int offset) const;

        int getOffset() const {
            return _offset;
        }

        int getNextOffset() const {
            return (_offset + 1) % (_maxOffset + 1);
        }

        int incrementOffset() {
            _offset += (_offset + 1) % (_maxOffset + 1);
            return _offset;
        }

        int getSum() const;

        std::string toString() const;

        friend std::ostream& operator<<(std::ostream& os, const CellMatrix& obj) {
            os << obj.toString();
            return os;
        }

    private:
        std::vector<uint64_t> _grid;
        int _rows, _columns;
        int _maxOffset = 1, _offset = 0;;
        uint8_t offset = 0;
    };
}

#endif //CELL_MATRIX_HPP