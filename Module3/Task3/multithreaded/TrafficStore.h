#ifndef TASK3_TRAFFICSTORE_H
#define TASK3_TRAFFICSTORE_H

#include <mutex>
#include <map>

#include "types.h"


class TrafficStore {
private:
    std::mutex write_mutex;

    std::map<local_hours, congestion_entry_t> congestion_map;

public:
    void add_entry(TrafficData &data) {
        // Work out what hour it fits into
        local_hours hour = std::chrono::time_point_cast<std::chrono::hours>(data.timestamp).time_since_epoch()

        // Initialise hour entry if it doesn't exist yet
        if (!this->congestion_map.contains(hour)) {
            const std::lock_guard<std::mutex> lock(this->write_mutex);

            this->congestion_map->try_emplace(hour);
        }

        // Initialise traffic slot if it doesn't exist yet
        if (!this->congestion_map[hour].contains(data.traffic_id)) {
            const std::lock_guard<std::mutex> lock(this->write_mutex);

            this->congestion_map[hour].try_emplace(data.traffic_id, 0);
        }
    }

    void print_entries() {

    }
};

#endif
