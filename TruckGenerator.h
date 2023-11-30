#include "simlib.h"
#include "Truck.h"

#ifndef TRUCK_GENERATOR_H
#define TRUCK_GENERATOR_H

class TruckGenerator : public Event {
    public: 
    
    TruckGenerator(
        Store * ramps,
        Store * aWorkers,
        Store * bWorkers
    );

    void Behavior() override;

    Store * r;
    Store * a;
    Store * b;
};

#endif