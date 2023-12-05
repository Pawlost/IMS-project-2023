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
#define TRUCK_ARRIVE_TIME 30

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

Stat rampWaitTime("Čekání na uvolnění rampy");

int loadingTrucksGenerated = 0;
int unloadingTrucksGenerated = 0;
int loadingDeliveryTrucksGenerated = 0;
int unloadingDeliveryTrucksGenerated = 0;

int deliveryTrucksProcessed = 0;
int administrationFinished = 0;
int goodsStockStored = 0;

void showHelp(const char *programName) {
  cerr << "Usage: " << programName << " <option(s)>\n"
       << "Options:\n"
       << "\t-z / --ramps AMOUNT\t\t Amount of open ramps in the warehouse.\n"
       << "\t-x / --fullTimeWorkers AMOUNT\t\t Amount of Full-Time workers "
          "working inside storage.\n"
       << "\t-y / --partTimeWorkers AMOUNT\t\t Amount of Part-Time workers "
          "working inside storage.\n"
       << "\t-v / --officeWorkers AMOUNT\t\t Amount of office workers managing "
          "storage administration.\n"
       << "\t-w / --forklifts AMOUNT\t\t Amount of forklifts available in the "
          "storage.\n\n"
       << "\t[-f / --file FILENAME]\t\t Save output to the file FILENAME.\n"
       << "\t-h / --help\tShow this help message.\n"
       << "Validation:\n"
       << "\t RAMP AMOUNT >= 1\n"
       << "\t FULL-TIME WORKER AMOUNT >= 2\n"
       << "\t PART-TIME WORKER AMOUNT >= 4\n"
       << "\t OFFICE WORKER AMOUNT >= 1\n"
       << "\t FOKLIFT AMOUNT >= 1\n"
       << endl;

  exit(0);
}

void showHelp(const char *programName, const string shortName,
              const string longName, int minValue) {
  cerr << "Wrong argument value " << shortName << " " << longName
       << ". Value should start from " << minValue << ". ";
  showHelp(programName);
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

int getOptionNumber(char **start, char *end[], const string &optionString,
                    const string &optionStringLong, int minValue) {
  char *option = getOptionValue(start, end, optionString, optionStringLong);

  if (!option || stoi(option) < minValue) {
    showHelp(start[0], optionString, optionStringLong, minValue);
  }

  return stoi(option);
}

void parseAllArguments(char **start, char **end) {
  if (findOptionString(start, end, "-h") ||
      findOptionString(start, end, "--help")) {
    showHelp(start[0]);
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

  option = getOptionNumber(start, end, "-v", "--officeWorkers", 1);
  officeWorkers = new Store("Úředníci", option);

  option = getOptionNumber(start, end, "-w", "--forklifts", 1);
  forklifts = new Store("Vysokozdvižné vozíky", option);
}

void QueueActivateFirst(Queue &q) {
  if (q.Length() > 0) {
    (q.GetFirst())->Activate();
  }
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

  void TryEnter(Store *store, int amount) {
    if (amount > 0) {
      Enter(*store, amount);
    }
  }

  void TryLeave(Store *store, int amount) {
    if (amount > 0) {
      Leave(*store, amount);
    }
  }

  void EnterStorage() {
    double time = Time;

    while (fullTimeWorkers->Free() < fullTimeWorkersAmount ||
           partTimeWorkers->Free() < partTimeWorkersAmount ||
           ramps->Free() < rampAmount || forklifts->Free() < forkliftAmount) {
      Into(storageQueue);
      Passivate();
    }

    rampWaitTime(Time - time);
    TryEnter(fullTimeWorkers, fullTimeWorkersAmount);
    TryEnter(partTimeWorkers, partTimeWorkersAmount);
    TryEnter(ramps, rampAmount);
    TryEnter(forklifts, forkliftAmount);
  }

  void LeaveStorage(int fullTimeWorkersAmountLeave = -1) {
    fullTimeWorkersAmountLeave = fullTimeWorkersAmountLeave > -1
                                     ? fullTimeWorkersAmountLeave
                                     : fullTimeWorkersAmount;

    TryLeave(fullTimeWorkers, fullTimeWorkersAmountLeave);
    TryLeave(partTimeWorkers, this->partTimeWorkersAmount);
    TryLeave(ramps, this->rampAmount);
    TryLeave(forklifts, this->forkliftAmount);

    QueueActivateFirst(storageQueue);
  }

  void EnterAdministration(Priority_t administrationPriority) {
    this->Priority = administrationPriority;
    while (officeWorkers->Free() < officeWorkersAmount) {
      Into(administrationQueue);
      Passivate();
    }

    TryEnter(*officeWorkers, officeWorkersAmount);
  }

  void LeaveAdministration() {
    TryLeave(*officeWorkers, officeWorkersAmount);

    QueueActivateFirst(administrationQueue);
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

class UnloadVehicleProcess : public VehicleProcess {
public:
  UnloadVehicleProcess(Priority_t p, int fullTimeWorkersAmount,
                       int partTimeWorkersAmount, int rampAmount,
                       int forkliftAmount, int officeWorkersAmount)
      : VehicleProcess(p, fullTimeWorkersAmount, partTimeWorkersAmount,
                       rampAmount, forkliftAmount, officeWorkersAmount) {}

  void AdministerUnloadedGoods() {
    EnterAdministration(1);
    Wait(Uniform(5, 25));
    Leave(*fullTimeWorkers, 1);
    administrationFinished++;
    LeaveAdministration();

    (new StockGoodProcess())->Activate();
  }
};

class UnloadingTruck : public UnloadVehicleProcess {
public:
  UnloadingTruck()
      : UnloadVehicleProcess(UNLOADING_TRUCK_PRIORITY, 2, 4, 1, 1, 1) {}

  void Behavior() {
    EnterStorage();
    Wait(Uniform(20, 45));
    LeaveStorage(1);

    AdministerUnloadedGoods();
  }
};

class UnloadingDeliveryTruck : public UnloadVehicleProcess {
public:
  UnloadingDeliveryTruck()
      : UnloadVehicleProcess(UNLOADING_DELIVERY_TRUCK_PRIORITY, 1, 2, 1, 0, 1) {
  }

  void Behavior() {

    EnterStorage();
    Wait(Uniform(15, 25));
    deliveryTrucksProcessed++;
    LeaveStorage(0);

    AdministerUnloadedGoods();
  }
};

class LoadingTruck : public VehicleProcess {
public:
  LoadingTruck() : VehicleProcess(LOADING_TRUCK_PRIORITY, 2, 4, 1, 1, 1) {}

  void Behavior() {
    EnterStorage();
    Wait(Uniform(25, 35));
    deliveryTrucksProcessed++;
    LeaveStorage();

    EnterAdministration(2);
    Wait(Exponential(10));
    administrationFinished++;
    LeaveAdministration();
  }
};

class LoadingDeliveryTruck : public VehicleProcess {
public:
  LoadingDeliveryTruck()
      : VehicleProcess(LOADING_DELIVERY_TRUCK_PRIORITY, 2, 4, 1, 1, 1) {}

  void Behavior() {
    EnterStorage();
    Wait(Uniform(20, 25));
    deliveryTrucksProcessed++;
    LeaveStorage();

    EnterAdministration(3);
    Wait(Exponential(10));
    administrationFinished++;
    LeaveAdministration();
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
        (new UnloadingDeliveryTruck())->Into(storageQueue);
        unloadingDeliveryTrucksGenerated++;
      } else {
        (new UnloadingTruck())->Into(storageQueue);
        unloadingTrucksGenerated++;
      }
    } else {

      // Loading
      if (Random() <= 0.3) {
        (new LoadingDeliveryTruck())->Into(storageQueue);
        loadingDeliveryTrucksGenerated++;
      } else {
        (new LoadingTruck())->Into(storageQueue);
        loadingTrucksGenerated++;
      }
    }

    QueueActivateFirst(storageQueue);

    double next = Time + Exponential(TRUCK_ARRIVE_TIME);
    Activate(next);
  }
};

int main(int argc, char *argv[]) {
  parseAllArguments(argv, argv + argc);

  for (int i = 0; i < argc; i++) {
    Print(argv[i]);
  }
  Print("\n");

  Init(0, SIMULATION_END_TIME); // Initialize simulation

  (new VehicleGenerator())->Activate();

  Run(); // Run simulation

  cerr << "Finished simulation, printing results" << endl;

  Print("\n================== Experiment 1 ==================\n\n");
  ramps->Output();
  rampWaitTime.Output();

  Print("\n\n================== Experiment 2 ==================\n\n");

  Print("\n\n================== Others ==================\n\n");
  Print("Počet vygenerovaných kamionů na vyložení: ");
  Print(unloadingTrucksGenerated);
  Print("\n");

  Print("Počet vygenerovaných kamionů na naložení: ");
  Print(loadingTrucksGenerated);
  Print("\n");

  Print("Počet vygenerovaných dodávek na vyložení: ");
  Print(unloadingDeliveryTrucksGenerated);
  Print("\n");

  Print("Počet vygenerovaných dodávek na naložení: ");
  Print(loadingDeliveryTrucksGenerated);
  Print("\n");

  Print("Počet vozidel celkem: ");
  Print(loadingDeliveryTrucksGenerated + unloadingTrucksGenerated + loadingTrucksGenerated + 
  unloadingDeliveryTrucksGenerated);
  Print("\n\n");

  fullTimeWorkers->Output();

  partTimeWorkers->Output();

  officeWorkers->Output();

  forklifts->Output();

  storageQueue.Output();

  administrationQueue.Output();

  cerr << "Done" << endl;
  return 0;
}