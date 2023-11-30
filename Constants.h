#include "simlib.h"
#include <iostream>
#include <algorithm>

#ifndef CONSTANTS_H

using namespace std;

#define CONSTANTS_H
#define SIMULATION_END_TIME 1000000

#define TRUCK_UNLOADING 0
#define TRUCK_LOADING 1

#define TRUCK_ACTIVATE_TIME_MEAN 60
#define A_WORKER_SERVICE_COUNT 1
#define B_WORKER_SERVICE_COUNT 2
#define A_WORKER_STORING_SERVICE_COUNT 2
#define B_WORKER_CONTROL_SERVICE_COUNT 1

class ProjectSettings {
public:

    int rampsFullCount = 0;
    int aWorkersFullCount = 0;
int bWorkersFullCount = 0;
int workersUnavailable = 0;
int truckCount = 0;
int truckService = 0;

int loadingCount = 0;
int unloadingCount = 0;

double totalServiceTime = 0;
double totalAWorkersTime = 0;
double totalBWorkersTime = 0;

Queue rampQueue = Queue("Cekani na rampu");
Queue serviceQueue = Queue("Cekani na obsluhu");

   // Delete copy constructor and assignment operator
   ProjectSettings(const ProjectSettings&) = delete;
   ProjectSettings& operator=(const ProjectSettings&) = delete;

   // Get the singleton instance
   static ProjectSettings& getInstance() {
        Print("HERE LMAO");
       static ProjectSettings instance; // Guaranteed to be destroyed.
                                // Instantiated on first use.
       return instance;
   }

private:
   ProjectSettings() {} // Constructor? (has been made private)
};
#endif