#ifndef TASK3_TRAFFICDATA_H
#define TASK3_TRAFFICDATA_H

#include <iostream>
#include <date/date.h>

#include "types.h"


// Stores a line of traffic data
struct TrafficData {
    traffic_timestamp timestamp;
    traffic_id_t traffic_id{};
    traffic_count_t traffic_count{};
};

// Extraction operator override to support getting traffic data
std::istream& operator>>(std::istream& in, struct TrafficData& trafficData) {
    struct TrafficData temp {};

    // Skip whitespace, then load in the timestamp and other info
    in >> std::ws >> date::parse("%F %T", temp.timestamp) >> temp.traffic_id >> temp.traffic_count;

    trafficData = std::move(temp);

    return in;
}

#endif //TASK3_TRAFFICDATA_H
