/*  FIT VUT - Modelování a simulace
**  SHO v logistice
**
**  Date: 2. 12. 2023
*/

#include "simlib.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>

using namespace std;

#define SIMULATION_END_TIME 1000000
#define TRUCK_ARRIVE_TIME 60

#define UNLOADING_TRUCK_PRIORITY 2
#define LOADING_TRUCK_PRIORITY 4

#define UNLOADING_DELIVERY_TRUCK_PRIORITY 3
#define LOADING_DELIVERY_TRUCK_PRIORITY 5

Store *fullTimeWorkers;
Store *partTimeWorkers;
Store *ramps;
Store *officeWorkers;
Store *forklifts;

Queue storageQueue("Fronta ke skladu");
Queue administrationQueue("Fronta na razítko");

int trucksProcessed = 0;
int deliveryTrucksProcessed = 0;
int administrationFinished = 0;
int goodsStockStored = 0;

void showHelp() {
  cerr << "Správné použití:" << endl;

  exit(0);
}

void showHelp(const string shortName, const string longName, int minValue) {
  cerr << "Špatný argument" << shortName << " " << longName
       << ". Hodnota by měla začínat od " << minValue << ". ";
  showHelp();
}

bool findOptionString(char *start[], char *end[], const string &optionString) {
  return find(start, end, optionString) != end;
}

char *getOptionValue(char *start[], char *end[], const string &optionString,
                     const string &optionStringLong) {
  /* Find option string and return option value */
  /* https://stackoverflow.com/questions/865668/parsing-command-line-arguments-in-c
   */
  char **itr = find(start, end, optionString);
  if (itr != end && ++itr != end)
    return *itr;

  char **itrLong = find(start, end, optionStringLong);
  if (itrLong != end && ++itrLong != end)
    return *itrLong;
  return NULL;
}

int getOptionNumber(char *start[], char *end[], const string &optionString,
                    const string &optionStringLong, int minValue) {
  char *option = getOptionValue(start, end, optionString, optionStringLong);

  if (!option || stoi(option) < minValue) {
    showHelp(optionString, optionStringLong, minValue);
  }

  return stoi(option);
}

void parseAllArguments(char **start, char **end) {
  if (findOptionString(start, end, "-h") ||
      findOptionString(start, end, "--help")) {
    showHelp();
  }
  char *outputFile = getOptionValue(start, end, "-f", "--file");
  if (outputFile) {
    SetOutput(outputFile);
  }

  int option = getOptionNumber(start, end, "-z", "--ramps", 1);
  ramps = new Store("Rampy", option);

  option = getOptionNumber(start, end, "-x", "--fullTimeWorkers", 2);
  fullTimeWorkers = new Store("Stálý pracovníci", option);

  option = getOptionNumber(start, end, "-y", "--partTimeWorkers", 4);
  partTimeWorkers = new Store("Brigádnící", option);

  option = getOptionNumber(start, end, "-v", "--officialWorkers", 1);
  officeWorkers = new Store("Úředníci", option);

  option = getOptionNumber(start, end, "-w", "--forklifts", 1);
  forklifts = new Store("Vysokozdvižné vozíky", option);
}

class VehicleProcess : public Process {
private:
  int fullTimeWorkersAmount;
  int partTimeWorkersAmount;
  int rampAmount;
  int forkliftAmount;
  int officeWorkersAmount;

public:
  VehicleProcess(Priority_t p, int fullTimeWorkersAmount,
                 int partTimeWorkersAmount, int rampAmount, int forkliftAmount,
                 int officeWorkersAmount)
      : Process(p) {
    this->fullTimeWorkersAmount = fullTimeWorkersAmount;
    this->partTimeWorkersAmount = partTimeWorkersAmount;
    this->rampAmount = rampAmount;
    this->forkliftAmount = forkliftAmount;
    this->officeWorkersAmount = officeWorkersAmount;
  }

  void EnterStorage() {
    while (fullTimeWorkers->Free() < fullTimeWorkersAmount &&
           partTimeWorkers->Free() < partTimeWorkersAmount &&
           ramps->Free() < rampAmount && forklifts->Free() < forkliftAmount) {
      Into(storageQueue);
      Passivate();
    }

    Enter(*fullTimeWorkers, fullTimeWorkersAmount);
    Enter(*partTimeWorkers, partTimeWorkersAmount);
    Enter(*ramps, rampAmount);
    Enter(*forklifts, forkliftAmount);
  }

  void LeaveStorage() {
    Leave(*partTimeWorkers, this->partTimeWorkersAmount);
    Leave(*ramps, this->rampAmount);
    Leave(*forklifts, this->forkliftAmount);

    if (storageQueue.Length() > 0) {
      (storageQueue.GetFirst())->Activate();
    }
  }

  void EnterAdministration() {
    while (officeWorkers->Free() < officeWorkersAmount) {
      Into(administrationQueue);
      Passivate();
    }

    Enter(*officeWorkers, officeWorkersAmount);
  }

  void LeaveAdministration() {
    Leave(*officeWorkers, officeWorkersAmount);

    if (administrationQueue.Length() > 0) {
      (administrationQueue.GetFirst())->Activate();
    }
  }
};

class StockGoodProcess : public Process {
public:
  StockGoodProcess() : Process() {}

  void Behavior() {
    const int partTimeWorksAmount = 2;
    const int fokliftsAmount = 1;
    while (partTimeWorkers->Free() < partTimeWorksAmount &&
           forklifts->Free() < fokliftsAmount) {
      Into(storageQueue);
      Passivate();
    }

    Enter(*partTimeWorkers, partTimeWorksAmount);
    Enter(*forklifts, fokliftsAmount);

    Wait(Uniform(10, 25));

    Leave(*partTimeWorkers, partTimeWorksAmount);
    Leave(*forklifts, fokliftsAmount);

    goodsStockStored++;

    if (storageQueue.Length() > 0) {
      (storageQueue.GetFirst())->Activate();
    }
  }
};

class UnloadingTruck : public VehicleProcess {
public:
  UnloadingTruck() : VehicleProcess(UNLOADING_TRUCK_PRIORITY, 2, 4, 1, 1, 1) {}

  void Behavior() {
    EnterStorage();
    Wait(1.0);
    Leave(*fullTimeWorkers, 1);
    trucksProcessed++;
    LeaveStorage();

    EnterAdministration();
    Wait(Uniform(5, 25));
    Leave(*fullTimeWorkers, 1);
    administrationFinished++;
    LeaveAdministration();

    (new StockGoodProcess())->Activate();
  }
};

class UnloadingDeliveryTruck : public VehicleProcess {
public:
  UnloadingDeliveryTruck() : VehicleProcess(UNLOADING_DELIVERY_TRUCK_PRIORITY
  
  ) {}

  void Behavior() {

  }
};

class LoadingTruck : public VehicleProcess {
public:
  LoadingTruck() : VehicleProcess(LOADING_TRUCK_PRIORITY
  
  ) {}

  void Behavior() {
  }
};

class LoadingDeliveryTruck : public VehicleProcess {
public:
  LoadingDeliveryTruck() : VehicleProcess(LOADING_DELIVERY_TRUCK_PRIORITY
  
  ) {}

  void Behavior() {
  }
};

class VehicleGenerator : public Event {
public:
  VehicleGenerator() : Event() {}
  void Behavior() {
    /*
        Periodically generate new Truck process.
        50% - truck loading
        50% - truck unloading
    */

    if (Random() <= 0.5) {

      // Unloading
      if (Random() <= 0.3) {
        (new UnloadingDeliveryTruck())->Activate();
      } else {
        (new UnloadingTruck())->Activate();
      }
    } else {

      // Loading
      if (Random() <= 0.3) {
        (new LoadingDeliveryTruck())->Activate();
      } else {
        (new LoadingTruck())->Activate();
      }
    }

    double next = Time + Exponential(TRUCK_ARRIVE_TIME);
    Activate(next);
  }
};

int main(int argc, char *argv[]) {
  parseAllArguments(argv, argv + argc);

  Init(0, SIMULATION_END_TIME); // Initialize simulation

  (new VehicleGenerator())->Activate();

  Run(); // Run simulation

  cerr << "Finished simulation, printing results" << endl;

  ramps->Output();
  fullTimeWorkers->Output();

  partTimeWorkers->Output();

  officialWorkers->Output();

  forklifts->Output();

  cerr << "Done" << endl;
  return 0;
}