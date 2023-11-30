#include "Truck.h"

ProjectSettings& projSet = ProjectSettings::getInstance();


void Truck::Service() {
        /* 
            Takes 1 professional and 2 ordinary storemen.
            Simulate truck unloading or loading.
        */
        double enterTime = Time;
        Enter( * a, A_WORKER_SERVICE_COUNT);
        Enter( * b, B_WORKER_SERVICE_COUNT);

        Wait(Uniform(20, 45)); // Unloading/Loading

        Leave( * a, A_WORKER_SERVICE_COUNT);
        Leave( * b, B_WORKER_SERVICE_COUNT);

        projSet.totalAWorkersTime += (Time - enterTime) * A_WORKER_SERVICE_COUNT / a -> Capacity();
        projSet.totalBWorkersTime += (Time - enterTime) * B_WORKER_SERVICE_COUNT / b -> Capacity();
        projSet.truckService++;
}

void Truck::Action(double enterTime) {
        /*
            Finds free storemen for unloading/loading.
            After the service is done, truck releases ramp.
        */
        Enter( * r, 1);

        if (a -> Full()) projSet.aWorkersFullCount++;
        if (b -> Full()) projSet.bWorkersFullCount++;
        if (!(a -> Free() >= A_WORKER_SERVICE_COUNT && b -> Free() >= B_WORKER_SERVICE_COUNT)) {
            // Stats
            projSet.workersUnavailable++;
        }
        while (!(a -> Free() >= A_WORKER_SERVICE_COUNT && b -> Free() >= B_WORKER_SERVICE_COUNT)) {
            projSet.serviceQueue.Insert(this);
            Passivate();

            break;
        }

        Service();

        if (projSet.serviceQueue.Length() > 0) {
            //printf("[%d] Aktivuje pro obsluhu...\n", celkem);
            (projSet.serviceQueue.GetFirst()) -> Activate();
        }

        Leave( * r, 1);

        if (serviceType == TRUCK_UNLOADING) {
            projSet.unloadingCount++;
            (new ControlService(b, a)) -> Activate(); //Activate goods control process
            //(new StoringService(a)) -> Activate(); //Activate storing process
        } else if (serviceType == TRUCK_LOADING) projSet.loadingCount++;

        if (projSet.rampQueue.Length() > 0) {
            (projSet.rampQueue.GetFirst()) -> Activate();
        }
    }
    
    void Truck::Behavior() {
        /*
            Truck finds the free ramp and waits for storemen to load/unload.   
        */
        projSet.truckCount++;
        double enterTime = Time;
        if (r -> Full())
            projSet.rampsFullCount++;

        while (r -> Full()) {
            projSet.rampQueue.Insert(this);
            Passivate();
            break;
        }
        Action(enterTime);
    }