#ifndef TASK3_TRAFFICCONSUMER_H
#define TASK3_TRAFFICCONSUMER_H

#include <memory>
#include <stdexcept>
#include <chrono>
#include <iostream>
#include <sstream>

#include "types.h"
#include "TrafficThread.h"


// Consumer of traffic data from the queue.
// Accepts traffic data then adds it to the congestion map
class TrafficConsumer: public TrafficThread {
private:
    // Pointers to different shared resources
    std::shared_ptr<congestion_map_t> congestion_map;
    std::shared_ptr<traffic_queue_t> queue;

public:
    TrafficConsumer(std::shared_ptr<congestion_map_t> congestion_map, std::shared_ptr<traffic_queue_t> queue) {
        this->congestion_map = std::move(congestion_map);
        this->queue = std::move(queue);
    }

    // Runs the consumer once, taking in one piece of data
    // Returns false when it should stop and true otherwise.
    bool run_once() {
        TrafficData data;

        // Pop in a loop until we can pop a piece of data off the queue.
        while (!this->queue->pop(data)) {
            std::this_thread::sleep_for(std::chrono::microseconds(1)); // Tiny wait so the OS can switch out as needed
        }

        // If the data is a stop op, then push it back to the queue for other consumers, and return false
        if (!data.running) {
            // Composes the message first to ensure thread safe printing
            std::stringstream msg;
            msg << "End op found, ending consumer" << std::endl;
            std::cout << msg.str();

            this->queue->push(data);
            return false;
        }

        // Composes the message first to ensure thread safe printing
        std::stringstream msg;
        msg << "Consumed: " << date::format("%F %T", data.timestamp) << ", " << data.traffic_id << ", "
            << data.traffic_count << std::endl;
        std::cout << msg.str();

        this->congestion_map->add_entry(data);

        return true;
    }

    // Entrypoint for thread, continuously consumes data
    void run() {
        while (this->run_once()) {
        }
    }
};


#endif //TASK3_TRAFFICCONSUMER_H
