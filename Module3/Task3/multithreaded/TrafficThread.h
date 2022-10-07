#ifndef TASK3_TRAFFICTHREAD_H
#define TASK3_TRAFFICTHREAD_H

#include <thread>


// Interface representing a runnable thread object
class TrafficThread {
private:
    std::thread thread;

public:
    // Thread entrypoint
    virtual void run() = 0;

    // Start the thread
    void start() {
        this->thread = std::thread(&TrafficThread::run, this);
    }

    // Wait for the thread to finish
    void join() {
        this->thread.join();
    }
};

#endif