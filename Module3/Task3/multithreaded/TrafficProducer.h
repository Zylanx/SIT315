#ifndef TASK3_TRAFFICPRODUCER_H
#define TASK3_TRAFFICPRODUCER_H

#include <memory>
#include <stdexcept>
#include <iostream>
#include <sstream>
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

    // Adds an end op to the queue to kill all the consumers
    void enqueue_end_op(std::stringstream &msg) {
        // Lock the stopped variable so only one producer will ever enqueue an end op
        const std::lock_guard<std::mutex> lock(TrafficProducer::stopped.stopped_mutex);

        // No producer has stopped yet.
        if (!TrafficProducer::stopped.stopped) {
            // Create an end op
            TrafficData end_op;
            end_op.running = false;

            // Push it to the queue and set the stop variable
            this->queue->push(std::move(end_op));
            TrafficProducer::stopped.stopped = true;
        } else {
            msg << " Consumers already stopped";
        }
    }

public:
    TrafficProducer(std::shared_ptr<TrafficFile> file, std::shared_ptr<traffic_queue_t> queue) {
        this->file = std::move(file);
        this->queue = std::move(queue);
    }

    // Runs the producer once
    // Returns false when it should stop, true otherwise
    bool run_once() {
        // Read a line, print it, and push it to the queue.
        try {
            TrafficData temp = this->file->read_data();

            // Composes the message first to ensure thread safe printing
            std::stringstream msg;
            msg << "Produced: " << date::format("%F %T", temp.timestamp) << ", " << temp.traffic_id << ", "
                << temp.traffic_count << std::endl;
            std::cout << msg.str();

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

        // Composes the message first to ensure thread safe printing
        std::stringstream msg;
        msg << "File has reached EOF, enqueuing end op.";

        // Add a stop message to the queue to kill the consumers
        this->enqueue_end_op(msg);

        msg << std::endl;
        std::cout << msg.str();

        return;
    }
};


AtomicStop TrafficProducer::stopped = {std::mutex(), false};


#endif
