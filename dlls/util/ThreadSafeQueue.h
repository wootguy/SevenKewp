#pragma once

#undef min
#undef max

#include <queue>
#include <mutex>
#include <condition_variable>

// A threadsafe-queue.
template <class T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue(void): q(), m(), c() {}

    ~ThreadSafeQueue(void) {}

    // Add an element to the queue.
    void enqueue(T t){
        std::lock_guard<std::mutex> lock(m);
        q.push(t);
        c.notify_one();
    }

    // Get the "front"-element.
    // If the queue is empty, return false.
    bool dequeue(T& output) {
        std::unique_lock<std::mutex> lock(m);
        if (q.empty()) {
            return false;
        }
        output = q.front();
        q.pop();
        return true;
    }

    int size() {
        std::lock_guard<std::mutex> lock(m);
        return q.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m);
        std::queue<T> empty;
        std::swap(q, empty);
    }

private:
    std::queue<T> q;
    mutable std::mutex m;
    std::condition_variable c;
};