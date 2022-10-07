#ifndef TASK3_TRAFFICSTORE_H
#define TASK3_TRAFFICSTORE_H

#include <mutex>
#include <map>

#include "types.h"


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

class TrafficStore {
private:
    std::mutex write_mutex;

    std::map<local_hours, congestion_entry_t> congestion_map;

public:
    // Adds an entry to the congestion map
    void add_entry(TrafficData &data) {
        // Work out what hour it fits into
        local_hours hour = std::chrono::time_point_cast<std::chrono::hours>(data.timestamp).time_since_epoch();

        // Lock the congestion_map for writing
        const std::lock_guard<std::mutex> lock(this->write_mutex);

        // Initialise hour entry if it doesn't exist yet
        this->congestion_map.try_emplace(hour);

        // Initialise traffic slot if it doesn't exist yet
        this->congestion_map[hour].try_emplace(data.traffic_id, 0);

        this->congestion_map[hour][data.traffic_id] += data.traffic_count;
    }

    // Prints out the congestion map, printing out the most congested intersections for each hour.
    void print_entries() {
        // Iterate through each hour in order
        for (const auto& element : this->congestion_map) {
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
};

#endif
