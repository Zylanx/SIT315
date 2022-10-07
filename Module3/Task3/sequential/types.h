#ifndef TASK3_TYPES_H
#define TASK3_TYPES_H

#include <map>
#include <queue>
#include <date/date.h>

// Forward Declaration
struct TrafficData;


// Type aliases for use through-out the project
using traffic_timestamp = std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;
using traffic_id_t = size_t;
using traffic_count_t = unsigned int;

using local_hours = std::chrono::hours;
using congestion_entry_t = std::map<traffic_id_t, traffic_count_t>;
using congestion_map_t = std::map<local_hours, congestion_entry_t>;

using traffic_queue_t = std::queue<TrafficData>;

#endif //TASK3_TYPES_H
