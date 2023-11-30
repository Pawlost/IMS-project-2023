#include "TruckGenerator.h"
#include <simlib.h>


 TruckGenerator::TruckGenerator (Store * ramps,
        Store * aWorkers,
        Store * bWorkers) : Event::Event() {
        r = ramps;
        a = aWorkers;
        b = bWorkers;
    }

    void TruckGenerator::Behavior() {
        /* 
            Periodically generate new Truck process.
            50% - truck loading
            50% - truck unloading
        */
        if (Random() <= 0.5)
            (new Truck(TRUCK_LOADING, r, a, b)) -> Activate();
        else
            (new Truck(TRUCK_UNLOADING, r, a, b)) -> Activate();

        double next = Time + Exponential(TRUCK_ACTIVATE_TIME_MEAN);
        Event::Activate(next);
    }