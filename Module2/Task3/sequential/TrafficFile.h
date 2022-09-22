#ifndef TASK3_TRAFFICFILE_H
#define TASK3_TRAFFICFILE_H

#include <string>
#include <fstream>

#include "TrafficData.h"



class TrafficFile {
private:
    std::ifstream file;

public:
    explicit TrafficFile(std::string filename) {
        this->file.open(filename);
    }

    ~TrafficFile() {
        this->file.close();
    }

    TrafficData read_data() {
        TrafficData newData;
        this->file >> newData;

        return newData;
    }
};


#endif //TASK3_TRAFFICFILE_H
