#ifndef TASK3_TRAFFICCONSUMER_H
#define TASK3_TRAFFICCONSUMER_H

#include <chrono>
#include <memory>
#include <utility>

#include "types.h"


class TrafficConsumer {
private:
    std::shared_ptr<congestion_map_t> congestion_map;
    std::shared_ptr<traffic_queue_t> queue;

public:
    TrafficConsumer(std::shared_ptr<congestion_map_t> congestion_map, std::shared_ptr<traffic_queue_t> queue) {
        this->congestion_map = std::move(congestion_map);
        this->queue = std::move(queue);
    }

    void run_once() {

    }
};


#endif //TASK3_TRAFFICCONSUMER_H
