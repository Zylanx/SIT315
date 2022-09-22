#ifndef TASK3_TRAFFICPRODUCER_H
#define TASK3_TRAFFICPRODUCER_H

#include <memory>
#include <date/date.h>

#include "types.h"
#include "TrafficData.h"
#include "TrafficFile.h"


class TrafficProducer {
private:
    std::shared_ptr<TrafficFile> file;
    std::shared_ptr<traffic_queue_t> queue;

public:
    TrafficProducer(std::shared_ptr<TrafficFile> file, std::shared_ptr<traffic_queue_t> queue) {
        this->file = std::move(file);
        this->queue = std::move(queue);
    }

    void run_once() {
        TrafficData temp = this->file->read_data();
        std::cout << "Got: " << date::format("%F %T", temp.timestamp) << ", " << temp.traffic_id << ", " << temp.traffic_count;

        this->queue->push(temp);
    }
};


#endif //TASK3_TRAFFICPRODUCER_H
