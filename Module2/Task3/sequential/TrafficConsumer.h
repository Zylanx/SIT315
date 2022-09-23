#ifndef TASK3_TRAFFICCONSUMER_H
#define TASK3_TRAFFICCONSUMER_H

#include <memory>

#include "types.h"


// Consumer of traffic data from the queue.
// Accepts traffic data then adds it to the congestion map
class TrafficConsumer {
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
    void run_once() {
        // Pop the first piece of data off the queue.
        TrafficData data = this->queue->front();
        this->queue->pop();

        // Work out what hour it fits into
        local_hours hour = std::chrono::time_point_cast<std::chrono::hours>(data.timestamp).time_since_epoch();

        // If the hour slot doesn't exist, create it
        this->congestion_map->try_emplace(hour);

        // and if the traffic light slot is empty in that hour, fill it with zero
        if (!(*this->congestion_map)[hour].contains(data.traffic_id)) {
            (*this->congestion_map)[hour][data.traffic_id] = 0;
        }

        // Add the traffic count to the slot
        (*this->congestion_map)[hour][data.traffic_id] += data.traffic_count;
    }
};


#endif //TASK3_TRAFFICCONSUMER_H
