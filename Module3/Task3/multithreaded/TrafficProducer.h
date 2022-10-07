#ifndef TASK3_TRAFFICPRODUCER_H
#define TASK3_TRAFFICPRODUCER_H

#include <memory>
#include <stdexcept>
#include <date/date.h>

#include "types.h"
#include "TrafficData.h"
#include "TrafficFile.h"
#include "TrafficThread.h"
#include "AtomicStop.h"


// Produces traffic data from the file
class TrafficProducer : public TrafficThread {
private:
    // Shared resources
    std::shared_ptr<TrafficFile> file;
    std::shared_ptr<traffic_queue_t> queue;

    static AtomicStop stopped;

    void enqueue_end_op() {
        const std::lock_guard<std::mutex> lock(TrafficProducer::stopped.stopped_mutex);

        if (!TrafficProducer::stopped.stopped) {
            TrafficData emptyop;
            emptyop.running = false;

            this->queue->push(std::move(emptyop));
            TrafficProducer::stopped.stopped = true;
        }
    }

public:
    TrafficProducer(std::shared_ptr<TrafficFile> file, std::shared_ptr<traffic_queue_t> queue) {
        this->file = std::move(file);
        this->queue = std::move(queue);
    }

    // Runs the producer once
    bool run_once() {
        // Read a line, print it, and push it to the queue.
        try {
            TrafficData temp = this->file->read_data();
            std::cout << "Got: " << date::format("%F %T", temp.timestamp) << ", " << temp.traffic_id << ", "
                      << temp.traffic_count << std::endl;

            this->queue->push(temp);
            return true;
        } catch (...) {
            return false;
        }
    }

    // Entrypoint for thread, continuously produces data
    void run() {
        while (this->run_once()) {
        }

        std::cout << "File has reached EOF, enqueuing end op." << std::endl;
        this->enqueue_end_op();

        return;
    }
};


AtomicStop TrafficProducer::stopped = {std::mutex(), false};


#endif
