#include <memory>
#include <iterator>
#include <vector>
#include <thread>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <random>

#include "config.h"
#include "types.h"

#include "TrafficProducer.h"
#include "TrafficConsumer.h"
#include "TrafficThread.h"
#include "AtomicStop.h"
#include "TrafficStore.h"


// Set up the thread objects. Distributing the threads between producers and consumers according to the ratio configured
std::vector<std::unique_ptr<TrafficThread>> setup_thread_objects(std::shared_ptr<TrafficFile> file, std::shared_ptr<traffic_queue_t> queue, std::shared_ptr<congestion_map_t> congestion_map) {
    // Initialise the thread storage
    std::vector<std::unique_ptr<TrafficThread>> threads;
    auto threadCount = std::thread::hardware_concurrency() * THREAD_MULTIPLIER; // Get the number of hardware threads then multiply it, usually by 1.

    // Producer and consumer thread counts by ratio
    unsigned int producerCount = threadCount * PRODUCER_CONSUMER_RATIO;
    unsigned int consumerCount = threadCount * (1 - PRODUCER_CONSUMER_RATIO);

    std::cout << "Creating threads. Producers: " << producerCount << ", Consumers: " << consumerCount << std::endl;

    // Create the producer threads
    for (auto i = 0; i < producerCount; i++) {
        threads.push_back(std::unique_ptr<TrafficThread>(new TrafficProducer(file, queue)));
    }

    // Create the consumer threads
    for (auto i = 0; i < consumerCount; i++) {
        threads.push_back(std::unique_ptr<TrafficThread>(new TrafficConsumer(congestion_map, queue)));
    }

    // Randomly shuffle the threads so they start in a random order
    auto randomDevice = std::random_device {};
    auto randomEngine = std::default_random_engine { randomDevice() };
    std::shuffle(std::begin(threads), std::end(threads), randomEngine);

    return threads;
}

// Start all threads
void start_threads(std::vector<std::unique_ptr<TrafficThread>> &threads) {
    std::cout << "Starting threads..." << std::endl;

    for (auto &thread : threads) {
        thread->start();
    }
}

// Wait for all threads to finish
void wait_for_threads(std::vector<std::unique_ptr<TrafficThread>> &threads) {
    // Composes the message first to ensure thread safe printing
    std::stringstream msg;
    msg << "Joining on threads" << std::endl;
    std::cout << msg.str();

    for (auto &thread : threads) {
        thread->join();
    }
}

int main(int argc, char *argv[]) {
    // Create all the shared resources
    std::shared_ptr<TrafficFile> file;
    try {
        file = std::make_shared<TrafficFile>("test.txt"); // Load the test data
    } catch (std::ios_base::failure) {
        std::cerr << "Test file could not be found" << std::endl;
        return -1;
    }
    std::shared_ptr<traffic_queue_t> queue = std::make_shared<traffic_queue_t>(QUEUE_SIZE_BOUND);
    std::shared_ptr<congestion_map_t> congestion_map = std::make_shared<congestion_map_t>();

    // Create the producers and consumers
    std::vector<std::unique_ptr<TrafficThread>> threads = setup_thread_objects(file, queue, congestion_map);

    // Release the file and queue pointers
    file.reset();
    queue.reset();

    // Start the threads
    start_threads(threads);
    wait_for_threads(threads);

    std::cout << std::endl << "Processing Complete" << std::endl;

    // Print the final congestion map
    congestion_map->print_entries();
}