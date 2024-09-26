//
// Created by motst on 9/26/2024.
//

#include "ICellMatrix.hpp"

using namespace std;

namespace util {

    ICellMatrix::ICellMatrix(const int rows, const int columns, const int maxOffset, const int border)
            : _border(border),
              _rows(rows),
              _columns(columns),
              _maxOffset(maxOffset),
              _offset(0),
              _nextOffset(calculateNextOffset())
    {}

    void ICellMatrix::fillWithRandom(const int min, const int max)
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

    void ICellMatrix::fillFromVector(const vector<bool> & list) {
        for (int i = 0; i < this->rows(); i++) {
            for (int j = 0; j < this->columns(); j++)
                this->set(i, j, list.at(this->columns() * i + j));
        }
    }

    int ICellMatrix::getSum() const {
        int sum = 0;
        for (int i = 0; i < this->rows(); i++) {
            for (int j = 0; j < this->columns(); j++) {
                sum += this->get(i, j);
            }
        }
        return sum;
    }

    std::string ICellMatrix::toString() const {
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


} // util