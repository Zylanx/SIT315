#ifndef TASK3_TRAFFICFILE_H
#define TASK3_TRAFFICFILE_H

#include <string>
#include <fstream>

#include "TrafficData.h"


// Handler for the traffic file. Will later be used to handle locking
class TrafficFile {
private:
    std::ifstream file; // The file to read from

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
        TrafficData newData;

        this->file >> newData;

        return newData;
    }
};


#endif //TASK3_TRAFFICFILE_H
