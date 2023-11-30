    #include "ControlService.h"
ProjectSettings& projSet = ProjectSettings::getInstance();

    void ControlService::Behavior() {
        /* 
            Checking of received goods after unloading the truck.
            Checking is done by ordinary workers.
        */
        double enterTime = Time;

        Enter( * b, B_WORKER_CONTROL_SERVICE_COUNT);
        Wait(Uniform(15, 25));

        Leave( * b, B_WORKER_CONTROL_SERVICE_COUNT);
        (new StoringService(a)) -> Activate(); //Activate storing process
        projSet.totalBWorkersTime += (Time - enterTime) * B_WORKER_CONTROL_SERVICE_COUNT / b -> Capacity();
 }