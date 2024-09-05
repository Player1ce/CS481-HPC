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

// row major
class CellMatrix {
  public:
    CellMatrix(int rows, int columns, int maxOffset = 1) 
        : _rows(rows),
          _columns(columns),
          _offsetMax(maxOffset),
          _offset(0)
    {
        _grid.resize(((rows * columns + 63) * _offsetMax) / 64);
    }

    CellMatrix(int size, int maxOffset = 1) 
        : _rows(size),
          _columns(size),
          _offsetMax(maxOffset),
          _offset(0)

    {
        _grid.resize(((size * size + 63) * _offsetMax) / 64);
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
        if (offset > _offsetMax || offset < 0) {
            cout << "getRecieved invalid offset: " << offset << endl;
        }

        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "inside main set row: " << row << " column: " << column << " val: " << val << " offset: " << offset << endl;
        #endif

        int location = ((row * _columns + column) * _offsetMax) + offset;
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
        if (offset > _offsetMax || offset < 0) {
            cout << "getRecieved invalid offset: " << offset << endl;
        }

        #ifdef CELL_MATRIX_DEBUG_LOGGING
        cout << "in main get() row: " << row << " column: " << column << " offset: " << offset << endl;
        #endif


        int location = ((row * _columns + column) * _offsetMax) + offset;
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
    int _offsetMax = 1, _offset = 0;;
    uint8_t offset = 0;
};

int main(int argc, char** argv) {
    cout << "Hello World!" << endl;

    int size = 5;

    CellMatrix matrix = CellMatrix(size);

    #ifdef DEBUG_LOGGING
    cout << "constructor complete" << endl;
    #endif

    matrix.fillWithRandom();

    int sum = matrix.getSum();

    cout << matrix << endl;


    float percent = (sum / static_cast<float>(size*size)) * 100.0;
    cout << "percent: " << percent << endl;

    return 0;
}




struct Cell {
    int x, y;
    bool alive, change;
};


bool getCellUpdate(const vector<vector<bool>> &grid, int x, int y) {
    int neighborsAlive = -1;
    for (int i = -1; i <= 1; i++) {
        for (int k = -1; k <=1; k++) {
            neighborsAlive += grid.at(x + i).at(y + k); 
        }
        if (neighborsAlive >= 4) {
            break;
        }
    }

    if (neighborsAlive == -1) {
        cout << "error, neighbors alive was -1 at x: " << x << " y: " << y;
    }


    if (grid.at(x).at(y)) {    
        switch (neighborsAlive) {
            case (0):
            case (1):
                return false;
            break;

            case(2):
            case(3):
                return true;
            break;

            default:
                return false;
            break;
        }
    }
    else if (neighborsAlive == 3) {
        return true;
    }
    else {
        return false;
    }
}
