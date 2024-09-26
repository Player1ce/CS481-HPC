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
        explicit ThreadPool(const size_t numThreads) : _numWaiting(0), _numThreads(numThreads), _stop(false) {
            // create the threads and put them in a list
            for (size_t i = 0; i < numThreads; ++i) {
                _workers.emplace_back([this] {
                    // each thread waits on a signal until it is called, then pops a task off the list to run
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(_queueMutex);

                            if (_tasks.empty() && _numWaiting >= _numThreads - 1) {
                                _empty_signal.notify_all();
//                                    std::cout << "notifying" << std::endl;
                            }

                            // track the number of waiting threads, so we can react when all are done
                            _numWaiting++;

                            // wait until another task is queued
                            _condition.wait(lock, [this] { return _stop || !_tasks.empty(); });
                            _numWaiting--;

                            // notify the emptySignal if there are no threads running.
//                                std::cout << "task size: " << _tasks.size() << " numWaiting: " << _numWaiting << std::endl;


                            // exit the loop if all tasks have run and stop is true
                            if (_stop && _tasks.empty()) {
                                return;
                            }

                            // otherwise move a task from the list and delete its empty holder
                            task = std::move(_tasks.front());
                            _tasks.pop();
                        }
                        // run the task
                        task();
                    }
                });
            }
        }

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
        void notify_threads(size_t num_to_notify) {
            std::unique_lock<std::mutex> lock(_queueMutex);
            // make sure we don't try to notify more threads than we have or too many threads for our task list
            num_to_notify = std::min(std::min(num_to_notify, _tasks.size()), _numThreads);
            for (size_t i = 0; i < num_to_notify; ++i) {
                _condition.notify_one();
            }
        }

        /// \brief Stop all threads and join them
        ~ThreadPool() {
            {
                std::unique_lock<std::mutex> lock(_queueMutex);
                _stop = true;
            }
            _condition.notify_all();
            for (std::thread &worker: _workers) {
                worker.join();
            }
        }

        /// \brief Wait until the tasks list is empty and all tasks have been completed
        void waitTillEmpty() {
            std::unique_lock<std::mutex> lock(_empty_mutex);
            _empty_signal.wait(lock, [this]{ return (_tasks.empty() && _numWaiting >= _numThreads - 1); });
//                std::cout << "wait done" << std::endl;
        }

        [[nodiscard]] std::size_t getNumThreads() const {
            return _numThreads;
        }

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
