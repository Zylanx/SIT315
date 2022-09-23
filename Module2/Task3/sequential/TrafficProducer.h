#ifndef TASK3_TRAFFICPRODUCER_H
#define TASK3_TRAFFICPRODUCER_H

#include <memory>
#include <date/date.h>

#include "types.h"
#include "TrafficData.h"
#include "TrafficFile.h"


// Produces traffic data from the file
class TrafficProducer {
private:
    // Shared resources
    std::shared_ptr<TrafficFile> file;
    std::shared_ptr<traffic_queue_t> queue;

public:
    TrafficProducer(std::shared_ptr<TrafficFile> file, std::shared_ptr<traffic_queue_t> queue) {
        this->file = std::move(file);
        this->queue = std::move(queue);
    }

    // Runs the producer once
    void run_once() {
        // Read a line, print it, and push it to the queue.
        TrafficData temp = this->file->read_data();
        std::cout << "Got: " << date::format("%F %T", temp.timestamp) << ", " << temp.traffic_id << ", " << temp.traffic_count << std::endl;

        this->queue->push(temp);
    }
};


#endif //TASK3_TRAFFICPRODUCER_H
