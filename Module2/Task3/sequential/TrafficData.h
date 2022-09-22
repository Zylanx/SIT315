#ifndef TASK3_TRAFFICDATA_H
#define TASK3_TRAFFICDATA_H

#include <iostream>
#include <date/date.h>

#include "types.h"


struct TrafficData {
    traffic_timestamp timestamp;
    traffic_id_t traffic_id;
    traffic_count_t traffic_count;
};

std::istream& operator>>(std::istream& in, struct TrafficData& trafficData) {
    struct TrafficData temp;
    in >> date::parse("%F %T", temp.timestamp) >> temp.traffic_id >> temp.traffic_count;

    trafficData = std::move(temp);

    return in;
}

#endif //TASK3_TRAFFICDATA_H
