#include <memory>
#include <iterator>

#include "config.h"
#include "types.h"

#include "TrafficProducer.h"
#include "TrafficConsumer.h"


#pragma region "Flipping"
// The map flipping code was taken from https://stackoverflow.com/a/5056797
// No changes were made, all credit goes to Oliver Charlesworth

// Flips a maps pair
template<typename A, typename B>
std::pair<B,A> flip_pair(const std::pair<A,B> &p)
{
    return std::pair<B,A>(p.second, p.first);
}

// flips all the pairs in a map, turning it into a multimap
template<typename A, typename B>
std::multimap<B,A> flip_map(const std::map<A,B> &src)
{
    std::multimap<B,A> dst;
    std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()),
                   flip_pair<A,B>);
    return dst;
}
#pragma endregion


// Prints out the congestion map, printing out the most congested intersections for each hour.
void print_congestion_map(congestion_map_t const& congestionMap) {
    // Iterate through each hour in order
    for (const auto& element : congestionMap) {
        // Print the hour
        std::cout << "Hour: " << element.first.count() << std::endl;

        // Get the hours entry and flip it, to sort the traffic lights in reverse
        congestion_entry_t entry = element.second;
        std::multimap<traffic_count_t, traffic_id_t> flipped = flip_map(entry);

        // Now iterate through the first TOP_INTERSECTIONS lights, printing them out.
        auto traffic_element = flipped.rbegin();
        for (auto i = 0; i < TOP_INTERSECTIONS && traffic_element != flipped.rend(); i++) {
            std::cout << "\t" << "Light ID: " << traffic_element->second << ", Count: " << traffic_element->first << std::endl;
            traffic_element++;
        }
        std::cout << std::endl;
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
    std::shared_ptr<traffic_queue_t> queue = std::make_shared<traffic_queue_t>();
    std::shared_ptr<congestion_map_t> congestion_map = std::make_shared<congestion_map_t>();

    // Create the producer and consumer
    TrafficProducer producer(file, queue);
    TrafficConsumer consumer(congestion_map, queue);

    // Release the file and queue pointers
    file.reset();
    queue.reset();

    // Loop the producer and consumer, catching when the file runs out
    try {
        while (true) {
            producer.run_once();
            consumer.run_once();
        }
    } catch (const std::ios_base::failure) { }

    // Print the final congestion map
    print_congestion_map(*congestion_map);
}