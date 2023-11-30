#include "simlib.h"
#include "Constants.h"
#include "ControlService.h"

#ifndef TRUCK_H
#define TRUCK_H

class Truck: public Process {
    public: 
    
    Truck(
        unsigned int type,
        Store * ramps,
        Store * aWorkers,
        Store * bWorkers
    ): Process() {
        serviceType = type;
        r = ramps;
        a = aWorkers;
        b = bWorkers;
    }

    unsigned int serviceType;
    Store * r;
    Store * a;
    Store * b;

    void Service();
    void Action(double enterTime);
    void Behavior() override;
};

#endif