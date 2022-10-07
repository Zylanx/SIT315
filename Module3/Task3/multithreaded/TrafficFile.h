#ifndef TASK3_TRAFFICFILE_H
#define TASK3_TRAFFICFILE_H

#include <string>
#include <fstream>
#include <mutex>

#include "TrafficData.h"


// Handler for the traffic file. Will later be used to handle locking
class TrafficFile {
private:
    std::ifstream file; // The file to read from
    std::mutex mutex; // The mutex that protects from multiple threads reading at once.

public:
    explicit TrafficFile(std::string const& filename) {
        this->file.exceptions(std::ifstream::failbit);
        this->file.open(filename.c_str());
    }

    ~TrafficFile() {
        this->file.close();
    }

    // Extract a line of traffic data from the file
    TrafficData read_data() {
        // Guard this section, locking the file until the scope is left
        const std::lock_guard<std::mutex> lock(this->mutex);
        TrafficData newData;

        this->file >> newData;

        return newData;
    }
};


#endif //TASK3_TRAFFICFILE_H
