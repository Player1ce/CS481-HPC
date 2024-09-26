//
// Created by motst on 9/26/2024.
//

#ifndef CS481_HPC_ICELLMATRIX_HPP
#define CS481_HPC_ICELLMATRIX_HPP

#include <functional>
#include <sstream>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>

namespace util {

    class ICellMatrix {
    public:

        ICellMatrix(int rows, int columns, int maxOffset, int border = 0);

        virtual ~ICellMatrix() {
            std::cout << "Base destructor called" << std::endl;
        }

        void fillWithRandom(int min = 0, int max = 1);

        void fillFromVector(const std::vector<bool> & list);

        [[nodiscard]] inline int rows() const {
            return _rows;
        }

        [[nodiscard]] inline int columns() const {
            return _columns;
        }

        virtual bool set(int row, int column, bool val) = 0;

        virtual bool set(int row, int column, bool val, int offset) = 0;

        [[nodiscard]] virtual bool get(int row, int column) const = 0;

        [[nodiscard]] virtual bool get(int row, int column, int offset) const = 0;

        [[nodiscard]] virtual int getVerticalWindow(int row, int column) const = 0;

        [[nodiscard]] virtual int getVerticalWindow(int row, int column, int offset) const = 0;

        [[nodiscard]] inline int getOffset() const {
            return _offset;
        }

        [[nodiscard]] inline int getNextOffset() const {
            return _nextOffset;
        }

        [[nodiscard]] inline bool isSquare() const {
            return this->rows() == this->columns();
        }


        int inline incrementOffset() // NOLINT(*-use-nodiscard)
        {
        _offset = getNextOffset();
        _nextOffset = calculateNextOffset();
        return _offset;
        }

        [[nodiscard]] int getSum() const;

        [[nodiscard]] std::string toString() const;

        friend std::ostream& operator<<(std::ostream& os, const ICellMatrix& obj) {
            os << obj.toString();
            return os;
        }

    protected:
        [[nodiscard]] inline int calculateNextOffset() const {
            return (_offset + 1) % (_maxOffset + 1);
        }

        int _border; ///< the size of border to have around the matrix. A size of 1 will give a border of 1 cell
                     ///< around the entire matrix

        int _rows, _columns;
        int _maxOffset, _offset, _nextOffset;
    };

} // util

#endif //CS481_HPC_ICELLMATRIX_HPP
