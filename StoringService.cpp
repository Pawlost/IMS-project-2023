#include "StoringService.h"

ProjectSettings& projSet = ProjectSettings::getInstance();

void StoringService::Behavior() {
        /*
            Storage of recieved goods after unload.
            Done by professional workers.
        */
        double enterTime = Time;

        Enter( * a, A_WORKER_STORING_SERVICE_COUNT);

        Wait(Uniform(25, 35));
        Leave( * a, A_WORKER_STORING_SERVICE_COUNT);
        projSet.totalAWorkersTime += (Time - enterTime) * A_WORKER_STORING_SERVICE_COUNT / a -> Capacity();
}