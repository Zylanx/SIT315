#ifndef TASK3_ATOMICSTOP_H
#define TASK3_ATOMICSTOP_H

#include <mutex>


struct AtomicStop {
    std::mutex stopped_mutex;
    std::atomic<bool> stopped;
};

#endif
