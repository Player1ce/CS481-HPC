//
// Created by marcelo on 9/12/24.
//

#ifndef CS481_HPC_FIXEDSIZEQUEUE_HPP
#define CS481_HPC_FIXEDSIZEQUEUE_HPP

#include <queue>

#include <array>
#include <numeric>

namespace util {

    template<typename T, std::size_t Size = 3>
    class FixedSizeQueue {
    public:
        static_assert(std::is_same_v<decltype(std::declval<T>() + std::declval<T>()), T>,
                      "Type T must support the + operator");

        explicit FixedSizeQueue(const bool useRunningSum = false)
            : _useRunningSum(useRunningSum)
        {}

        void push(const T& value) {
            data[tail] = value;
            if (isFull) {
                head = (head + 1) % Size;
            }
            tail = (tail + 1) % Size;
            isFull = head == tail;

            if (_useRunningSum) {
                _runningSum = _runningSum + value;
            }
        }

        [[nodiscard]] T front() const {
            return data[head];
        }

        void pop() {
            if (!empty()) {

                if (_useRunningSum) {
                    _runningSum = _runningSum - data[tail];
                }

                head = (head + 1) % Size;
                isFull = false;
            }
        }

        [[nodiscard]] bool empty() const {
            return (!isFull && (head == tail));
        }

        [[nodiscard]] bool full() const {
            return isFull;
        }

        T sum() const {
            if (empty()) {
                return T(); // Return default-constructed T (usually 0 for numeric types)
            }

            if (_useRunningSum) {
                return _runningSum;
            } else {
                if (isFull) {
                    return std::accumulate(data.begin(), data.end(), T());
                }

                if (tail > head) {
                    return std::accumulate(data.begin() + head, data.begin() + tail, T());
                } else {
                    T sum = std::accumulate(data.begin() + head, data.end(), T());
                    sum += std::accumulate(data.begin(), data.begin() + tail, T());
                    return sum;
                }
            }
        }

    private:
        std::array<T, Size> data;
        bool _useRunningSum;
        T _runningSum;
        std::size_t head = 0;
        std::size_t tail = 0;
        bool isFull = false;

    };
}

#endif //CS481_HPC_FIXEDSIZEQUEUE_HPP
