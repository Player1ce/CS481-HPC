#include <iostream>
#include <vector>
#include <array>
#include <bitset>
#include <random>
#include <chrono>
#include <boost/dynamic_bitset.hpp>

using namespace std;

#define DEBUG_LOGGING
// #define CELL_MATRIX_DEBUG_LOGGING
// #define CELL_UPDATE_DEBUG_LOGGING

// row major
class CellMatrix {
  public:
    CellMatrix(int rows, int columns, int maxOffset = 1) 
        : _rows(rows),
          _columns(columns),
          _maxOffset(maxOffset),
          _offset(0)
    {
        _grid.resize(((rows * columns + 63) * (_maxOffset * 1)) / 64);
    }

    CellMatrix(int size, int maxOffset = 1) 
        : _rows(size),
          _columns(size),
          _maxOffset(maxOffset),
          _offset(0)

    {
        _grid.resize(((size * size + 63) * (_maxOffset + 1)) / 64);
    }

    void fillWithRandom(int min= 0, int max = 1) {
        // Create a random number generator
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator(seed);

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

    void setFromVector(vector<bool> list) {
        for (int i = 0; i < this->rows(); i++) {
            for (int j = 0; j < this->columns(); j++)
                this->set(i, j, list.at(this->columns() * i + j));
        }
    }

    bool isSquare() const {
        return _rows == _columns;
    }

    int rows() const {
        return _rows;
    }

    int columns() const {
        return _columns;
    }

    void set(int row, int column, bool val) {
        #ifdef CELL_MATRIX_DEBUG_LOGGING
            cout << "outside set" << endl;
        #endif

        set(row, column, val, _offset);
    }

    void set(int row, int column, bool val, int offset) {
        if (offset > _maxOffset || offset < 0) {
            cout << "getRecieved invalid offset: " << offset << endl;
        }

        // create an infinite border of zeroes around the grid.
        if (row < 0 || row > _columns || column < 0 || column > _rows) {
            cout << "Error: Attempting to set out of range element will do nothing." << endl;
            return;
        }

        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "inside main set row: " << row << " column: " << column << " val: " << val << " offset: " << offset << endl;
        #endif

        int location = ((row * _columns + column) * (_maxOffset + 1)) + offset;
        int index = location / 64;
        int bit = location % 64;

        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "data in main set location: " << location << " index: " << index << " bit: " << bit << endl;
        #endif

        if (val) {
            _grid.at(index) |= (static_cast<uint64_t>(1) << bit);

            #ifdef CELL_MATRIX_DEBUG_LOGGING
            cout << "raw byte: " << std::bitset<64>(_grid.at(index)) << endl;
            #endif 
        }
        else
            _grid.at(index) &= ~(static_cast<uint64_t>(1) << bit);
    }

    
    bool get(int row, int column) const {
        return get(row, column, _offset);
    }
    

    bool get(int row, int column, int offset) const {
        if (offset > _maxOffset || offset < 0) {
            cout << "getRecieved invalid offset: " << offset << endl;
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

        int location = ((row * _columns + column) * (_maxOffset + 1)) + offset;
        int index = location / 64;
        int bit = location % 64;

        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "data in main get - location: " << location << " index: " << index << " bit: " << bit << endl;
        #endif

        
        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "raw byte: " << std::bitset<64>(_grid.at(index)) << endl;
        cout << "raw byte after processing: " << std::bitset<64>((_grid.at(index) & (static_cast<uint64_t>(1) << bit))) << endl;
        cout << "value after processing: " << (_grid.at(index) & (static_cast<uint64_t>(1) << bit)) << endl;
        #endif 

        return (_grid.at(index) & (static_cast<uint64_t>(1) << bit)) >= 1;
    }

    int getOffset() {
        return _offset;
    }

    int getNextOffset() {
        return (_offset + 1) % (_maxOffset + 1);
    }

    int incrementOffset() {
        _offset += (_offset + 1) % (_maxOffset + 1);
        return _offset;
    }

    int getSum() {
        int sum = 0;
        for (int i = 0; i < this->rows(); i++) {
            for (int j = 0; j < this->columns(); j++) {
                sum += this->get(i, j);
            }
        }
        return sum;
    }

    std::string toString() const {
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

    friend std::ostream& operator<<(std::ostream& os, const CellMatrix& obj) {
        os << obj.toString();
        return os;
    }
    
  private:
    vector<uint64_t> _grid;
    int _rows, _columns;
    int _maxOffset = 1, _offset = 0;;
    uint8_t offset = 0;
};


bool getCellUpdate(CellMatrix &grid, int row, int column) {
    int neighborsAlive = -grid.get(row, column);

    #ifdef CELL_UPDATE_DEBUG_LOGGING
    cout << "[" << endl;
    #endif

    for (int i = -1; i <= 1; i++) {
        #ifdef CELL_UPDATE_DEBUG_LOGGING
        cout << "[";
        #endif

        for (int j = -1; j <=1; j++) {
            #ifdef CELL_UPDATE_DEBUG_LOGGING
            cout << " " << grid.get(row + i, column + j);
            #endif

            neighborsAlive += grid.get(row + i, column + j); 
        }

        #ifdef CELL_UPDATE_DEBUG_LOGGING
        cout << "]" << endl;
        #endif

        if (neighborsAlive >= 4) {
            break;
        }
    }

    #ifdef CELL_UPDATE_DEBUG_LOGGING
    cout << "]" << endl;

    cout << "val: " << neighborsAlive << endl;
    #endif

    switch (neighborsAlive) {
            case (-1):
            case (0):
            case (1):
                return false;
            break;

            case(2):
                return grid.get(row, column) == 1;
            break;

            case(3):
                return true;
            break;

            default:
                return false;
            break;
    }
}

void updateCells(CellMatrix &matrix) {
    int nextOffset = matrix.getNextOffset();
    for (int i = 0; i < matrix.rows(); i++) {
        for (int j = 0; j < matrix.columns(); j++) {
            matrix.set(i, j, getCellUpdate(matrix, i, j), nextOffset);
        }
    }
    matrix.incrementOffset();
}

int main(int argc, char** argv) {
    cout << "Hello World!" << endl;

    int size = 5;
    int iterations = 1000;
    int printThreshold = 101;

    CellMatrix matrix = CellMatrix(size);

    #ifdef DEBUG_LOGGING
    cout << "constructor complete" << endl;
    #endif

    matrix.fillWithRandom();

    // vector<bool> initializer = {
    //     true, true, false,
    //     true, true, false,
    //     false, false, false
    // };

    // matrix.setFromVector(initializer);

    int sum = matrix.getSum();

    if (size < printThreshold) {
        cout << matrix << endl;
    }

    for (int i = 0; i < iterations; i++) {
        updateCells(matrix);
    }

    if (size < printThreshold) {
        cout << "end matrix " << endl;
        cout << matrix << endl;
    }

    float percent = (sum / static_cast<float>(size*size)) * 100.0;
    cout << "percent: " << percent << endl;

    return 0;
}