#include "simlib.h"
#include "Constants.h"
#include "StoringService.h"

#ifndef CONTROL_SERVICE_H 
#define CONTROL_SERVICE_H
class ControlService : public Process {
    public: ControlService(Store * bWorkers, Store * aWorkers): Process() {
        b = bWorkers;
        a = aWorkers;
    }

    void Behavior() override;

    Store * b;
    Store * a;
};
#endif