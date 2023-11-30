#include "simlib.h"
#include "Constants.h"

#ifndef STORING_SERVICE_H 
#define STORING_SERVICE_H

class StoringService: public Process {
    public: StoringService(
        Store * aWorkers
    ): Process() {
        a = aWorkers;
    }

    void Behavior() override;

    Store * a;
};

#endif