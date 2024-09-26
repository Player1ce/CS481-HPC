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
            : _useRunningSum(useRunningSum),
            _runningSum()
        {}

        void resetQueue() {
            while (!this->empty()) {
                this->pop();
            }
        }

        void push(const T& value) {
            if (_useRunningSum && isFull) {
                _runningSum += value - data[tail];
            }
            else {
                _runningSum += value;
            }

            data[head] = value;

            if (isFull) {

                tail = (tail + 1) % Size;
            }

            head = (head + 1) % Size;
            isFull = tail == head;
        }

        [[nodiscard]] T front() const {
            return data[tail];
        }

        void pop() {
            if (!empty()) {

                if (_useRunningSum) {
                    _runningSum -= data[tail];
                }

                tail = (tail + 1) % Size;
                isFull = false;
            }
        }

        [[nodiscard]] bool empty() const {
            return (!isFull && (tail == head));
        }

        [[nodiscard]] bool full() const {
            return isFull;
        }

        T sum() const {
            if (empty()) {
                return T(); // Return default-constructed T (usually 0 for numeric types)
            }
            else if (_useRunningSum) {
                return _runningSum;
            }
            else {
                if (isFull) {
                    return std::accumulate(data.begin(), data.end(), T());
                }

                if (head > tail) {
                    return std::accumulate(data.begin() + tail, data.begin() + head, T());
                } else {
                    T sum = std::accumulate(data.begin() + tail, data.end(), T());
                    sum += std::accumulate(data.begin(), data.begin() + head, T());
                    return sum;
                }
            }
        }

    private:
        std::array<T, Size> data;
        bool _useRunningSum;
        T _runningSum;
        std::size_t tail = 0;
        std::size_t head = 0;
        bool isFull = false;

    };
}

#endif //CS481_HPC_FIXEDSIZEQUEUE_HPP
