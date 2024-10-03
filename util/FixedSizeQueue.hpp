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

        explicit FixedSizeQueue()
            : _runningSum()
        {}

        void resetQueue() {
            tail = head = 0;
            isFull = false;
            _runningSum = T();
        }

        void push(const T& value) {
            _runningSum += value - (isFull ? data[tail] : T());

            data[head] = value;
            head = (head + 1) % Size;

            if (isFull) {
                tail = (tail + 1) % Size;
            } else {
                isFull = head == tail;
            }
        }

        void pop() {
            if (!empty()) {

                _runningSum -= data[tail];

                tail = (tail + 1) % Size;
                isFull = false;
            }
        }

        [[nodiscard]] bool empty() const {
            return (!isFull && (tail == head));
        }

        T front() const {
            if (empty()) {
                return T();
            }
            return data[tail];
        }

        [[nodiscard]] bool full() const {
            return isFull;
        }

        T sum() const {
            if (empty()) {
                return T();
            }
            else {
                return _runningSum;
            }

            if (isFull) {
                return std::accumulate(data.begin(), data.end(), T());
            }
            if (head > tail) {
                return std::accumulate(data.begin() + tail, data.begin() + head, T());
            }
            return std::accumulate(data.begin() + tail, data.end(), T()) + std::accumulate(data.begin(), data.begin() + head, T());
        }

    private:
        std::array<T, Size> data;
        T _runningSum;
        std::size_t tail = 0;
        std::size_t head = 0;
        bool isFull = false;

    };
}

#endif //CS481_HPC_FIXEDSIZEQUEUE_HPP
