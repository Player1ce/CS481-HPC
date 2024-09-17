//
// Created by marcelo on 2/14/24.
//

#ifndef CS470_THREADPOOL_H
#define CS470_THREADPOOL_H

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

namespace util {
    class ThreadPool {
    public:
        /// \brief Create a threadpool with num_threads threads.
        /// \param numThreads The number of threads in the pool
        explicit ThreadPool(std::size_t numThreads);

        /// \brief Add a function to the queue to be run by the thread pool
        /// \tparam FunctionType The type of function to add to the list
        /// \param function The function to add to the task list
        template<typename FunctionType>
        void enqueue(FunctionType &&function) {
            {
                std::unique_lock<std::mutex> lock(_queueMutex);
                _tasks.emplace(std::forward<FunctionType>(function));
            }
            _condition.notify_one();
        }

        /// \brief Add a function to the queue to be run by the thread pool without notifying the thread pool
        /// \tparam FunctionType The type of function to add to the list
        /// \param function The function to add to the task list
        template<typename FunctionType>
        void silentEnqueue(FunctionType &&function) {
            {
                std::unique_lock<std::mutex> lock(_queueMutex);
                _tasks.emplace(std::forward<FunctionType>(function));
            }
        }

        /// \brief Notifies the number of threads that they should begin running tasks
        /// \param num_to_notify The number of threads to notify
        void notify_threads(size_t num_to_notify);

        /// \brief Stop all threads and join them
        ~ThreadPool();

        /// \brief Wait until the tasks list is empty and all tasks have been completed
        void waitTillEmpty();

        [[nodiscard]] std::size_t getNumThreads() const;

    private:
        std::vector<std::thread> _workers;
        std::queue<std::function<void()>> _tasks;
        std::mutex _queueMutex;
        std::condition_variable _condition;

        std::mutex _empty_mutex;
        std::condition_variable _empty_signal;

        int _numWaiting;

        const size_t _numThreads;

        bool _stop;
    };
} // util

#endif //CS470_THREADPOOL_H
