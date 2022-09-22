#include <memory>

#include "types.h"

#include "TrafficProducer.h"
#include "TrafficConsumer.h"


int main(int argc, char *argv[]) {
    TrafficFile file("test.txt");
    traffic_queue_t queue;

    TrafficProducer producer(std::make_shared<TrafficFile>(file), std::make_shared<traffic_queue_t>(queue));
}