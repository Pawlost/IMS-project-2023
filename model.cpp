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

#define SIMULATION_END_TIME 500000
#define TRUCK_ARRIVE_TIME 20

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

#define HISTOGRAM_STEP 10
#define HISTORGRAM_NUM_OF_INTERVALS 17

Stat rampWaitTime("Čekání na uvolnění rampy");
Stat fullTimeWorkerStat("Prac. X - Průměrná doba obsluhy");
Stat partTimeWorkerStat("Prac. Y - Průměrná doba obsluhy");
Stat officeWorkerStat("Prac. V - Průměrná doba obsluhy");
Stat forkliftStat("W - Průměrná doba obsluhy");

Histogram unloadingTruckSystemTime("Náklaďák na vyskladnění - čas v systému",
                                   HISTOGRAM_STEP, HISTOGRAM_STEP,
                                   HISTORGRAM_NUM_OF_INTERVALS);
Histogram
    unloadingDeliveryTruckSystemTime("Dodávka na vyskladnění - čas v systému",
                                     0, HISTOGRAM_STEP,
                                     HISTORGRAM_NUM_OF_INTERVALS);
Histogram loadingTruckSystemTime("Náklaďák na nakládku - čas v systému",
                                 HISTOGRAM_STEP, HISTOGRAM_STEP,
                                 HISTORGRAM_NUM_OF_INTERVALS);
Histogram loadingDeliveryTruckSystemTime("Dodávka na nakládku - čas v systému",
                                         HISTOGRAM_STEP, HISTOGRAM_STEP,
                                         HISTORGRAM_NUM_OF_INTERVALS);
Histogram storeGoodsSystemTime("Naskladnění zboží - čas v systému", 0,
                               HISTOGRAM_STEP, 4);

Histogram *partTimeWorkersHistogram;
Histogram *fullTimeWorkersHistogram;
Histogram *officeWorkersHistogram;
Histogram *forkliftsHistogram;

int loadingTrucksGenerated = 0;
int unloadingTrucksGenerated = 0;
int loadingDeliveryTrucksGenerated = 0;
int unloadingDeliveryTrucksGenerated = 0;
long notEnoughWorkers = 0;

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
  ramps = new Store("Rampy - Z", option);

  option = getOptionNumber(start, end, "-x", "--fullTimeWorkers", 2);
  fullTimeWorkers = new Store("Stálý pracovníci - X", option);
  fullTimeWorkersHistogram =
      new Histogram("Stálý pracovníci - X", 1, 1, option);

  option = getOptionNumber(start, end, "-y", "--partTimeWorkers", 4);
  partTimeWorkers = new Store("Brigádnící - Y", option);
  partTimeWorkersHistogram =
      new Histogram("Brigádníci - Y", 1, 1, option);

  option = getOptionNumber(start, end, "-v", "--officeWorkers", 1);
  officeWorkers = new Store("Úředníci - V", option);
  officeWorkersHistogram =
      new Histogram("Úředníci - V", 1, 1, option);

  option = getOptionNumber(start, end, "-w", "--forklifts", 1);
  forklifts = new Store("Vysokozdvižné vozíky - W", option);
  forkliftsHistogram = 
      new Histogram("Vysokozdvižné vozíky - W", 1, 1, option);
}

void QueueActivateFirst(Queue &q) {
  if (q.Length() > 0) {
    (q.GetFirst())->Activate();
  }
}

void MarkMultipleStatTimes(Stat &stat, double deltaTime, int amount) {
  for (int i = 0; i < amount; i++) {
    stat(deltaTime);
  }
}

class StorageProcess : public Process {
private:
  int fullTimeWorkersAmount;
  int partTimeWorkersAmount;
  int rampAmount;
  int forkliftAmount;
  int officeWorkersAmount;

public:
  StorageProcess(Priority_t p, int fullTimeWorkersAmount,
                 int partTimeWorkersAmount, int rampAmount, int forkliftAmount,
                 int officeWorkersAmount)
      : Process(p) {
    this->fullTimeWorkersAmount = fullTimeWorkersAmount;
    this->partTimeWorkersAmount = partTimeWorkersAmount;
    this->rampAmount = rampAmount;
    this->forkliftAmount = forkliftAmount;
    this->officeWorkersAmount = officeWorkersAmount;
  }

  void TryEnter(Store *store, int amount, Histogram *histogram = nullptr) {
    if (amount > 0) {
      Enter(*store, amount);
      if(histogram != nullptr){
        (*histogram)(store->Used());
      }
    }
  }

  void TryLeave(Store *store, int amount, Stat *stat = nullptr,
                double deltaTime = 0.0) {
    if (amount > 0) {
      Leave(*store, amount);

      if (stat != nullptr) {
        MarkMultipleStatTimes(*stat, deltaTime, amount);
      }
    }
  }

  void EnterStorage() {
    double time = Time;

    while (fullTimeWorkers->Free() < fullTimeWorkersAmount ||
           partTimeWorkers->Free() < partTimeWorkersAmount ||
           ramps->Free() < rampAmount || forklifts->Free() < forkliftAmount) {

      if (fullTimeWorkers->Free() < fullTimeWorkersAmount ||
          partTimeWorkers->Free() < partTimeWorkersAmount) {
        notEnoughWorkers++;
      }

      Into(storageQueue);
      Passivate();
    }

    rampWaitTime(Time - time);
    TryEnter(fullTimeWorkers, fullTimeWorkersAmount, fullTimeWorkersHistogram);
    TryEnter(partTimeWorkers, partTimeWorkersAmount, partTimeWorkersHistogram);
    TryEnter(ramps, rampAmount);
    TryEnter(forklifts, forkliftAmount, forkliftsHistogram);
  }

  void LeaveStorage(double deltaTime, int fullTimeWorkersAmountLeave = -1) {
    fullTimeWorkersAmountLeave = fullTimeWorkersAmountLeave > -1
                                     ? fullTimeWorkersAmountLeave
                                     : fullTimeWorkersAmount;

    TryLeave(fullTimeWorkers, fullTimeWorkersAmountLeave, &fullTimeWorkerStat,
             deltaTime);
    TryLeave(partTimeWorkers, this->partTimeWorkersAmount, &partTimeWorkerStat,
             deltaTime);
    TryLeave(ramps, this->rampAmount);
    TryLeave(forklifts, this->forkliftAmount, &forkliftStat, deltaTime);

    QueueActivateFirst(storageQueue);
  }

  void EnterAdministration(Priority_t administrationPriority) {
    this->Priority = administrationPriority;

    if (administrationQueue.Length() > 0) {
      Into(administrationQueue);
      Passivate();
    }

    while (officeWorkers->Free() < officeWorkersAmount) {
      Into(administrationQueue);
      Passivate();
    }

    TryEnter(*officeWorkers, officeWorkersAmount, officeWorkersHistogram);
  }

  void LeaveAdministration(double deltaTime) {
    TryLeave(*officeWorkers, officeWorkersAmount, &officeWorkerStat, deltaTime);

    QueueActivateFirst(administrationQueue);
  }
};

class StockGoodProcess : public StorageProcess {
public:
  StockGoodProcess() : StorageProcess(0, 0, 2,
  0, 1, 0) {}

  void Behavior() {
    double time = Time;

    EnterStorage();
    Wait(Uniform(10, 25));
    double dt = Time - time;
    LeaveStorage(dt);

    storeGoodsSystemTime(Time - time);
    QueueActivateFirst(storageQueue);
  }
};

class UnloadVehicleProcess : public StorageProcess {
public:
  UnloadVehicleProcess(Priority_t p, int fullTimeWorkersAmount,
                       int partTimeWorkersAmount, int rampAmount,
                       int forkliftAmount, int officeWorkersAmount)
      : StorageProcess(p, fullTimeWorkersAmount, partTimeWorkersAmount,
                       rampAmount, forkliftAmount, officeWorkersAmount) {}

  void AdministerUnloadedGoods() {
    double time = Time;
    EnterAdministration(1);
    Wait(Uniform(5, 25));
    double dt = Time - time;
    fullTimeWorkerStat(dt);
    Leave(*fullTimeWorkers, 1);
    LeaveAdministration(dt);

    (new StockGoodProcess())->Into(storageQueue);
    QueueActivateFirst(storageQueue);
  }
};

class UnloadingTruck : public UnloadVehicleProcess {
public:
  UnloadingTruck()
      : UnloadVehicleProcess(UNLOADING_TRUCK_PRIORITY, 2, 4, 1, 1, 1) {}

  void Behavior() {
    double time = Time;
    EnterStorage();
    Wait(Uniform(20, 45));

    double dt = Time - time;
    LeaveStorage(dt, 1);
    unloadingTruckSystemTime(dt);

    AdministerUnloadedGoods();
  }
};

class UnloadingDeliveryTruck : public UnloadVehicleProcess {
public:
  UnloadingDeliveryTruck()
      : UnloadVehicleProcess(UNLOADING_DELIVERY_TRUCK_PRIORITY, 1, 2, 1, 0, 1) {
  }

  void Behavior() {
    double time = Time;
    EnterStorage();
    Wait(Uniform(15, 25));
    double dt = Time - time;
    partTimeWorkerStat(dt);
    LeaveStorage(dt, 0);

    unloadingDeliveryTruckSystemTime(Time - time);

    AdministerUnloadedGoods();
  }
};

class LoadingTruck : public StorageProcess {
public:
  LoadingTruck() : StorageProcess(LOADING_TRUCK_PRIORITY, 2, 4, 1, 1, 1) {}

  void Behavior() {
    double time = Time;
    EnterStorage();
    Wait(Uniform(25, 35));
    double dt = Time - time;
    fullTimeWorkerStat(dt);
    partTimeWorkerStat(dt);
    LeaveStorage(dt);

    EnterAdministration(2);
    Wait(Exponential(10));
    dt = Time - time;
    officeWorkerStat(dt);
    loadingTruckSystemTime(dt);
    LeaveAdministration(dt);
  }
};

class LoadingDeliveryTruck : public StorageProcess {
public:
  LoadingDeliveryTruck()
      : StorageProcess(LOADING_DELIVERY_TRUCK_PRIORITY, 2, 4, 1, 1, 1) {}

  void Behavior() {
    double time = Time;
    EnterStorage();
    Wait(Uniform(20, 25));
    double dt = Time - time;
    fullTimeWorkerStat(dt);
    partTimeWorkerStat(dt);
    LeaveStorage(dt);

    EnterAdministration(3);
    Wait(Exponential(10));
    dt = Time - time;
    officeWorkerStat(dt);
    loadingDeliveryTruckSystemTime(dt);
    LeaveAdministration(dt);
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

  fullTimeWorkers->Output();
  partTimeWorkers->Output();

  fullTimeWorkersHistogram->Output();
  partTimeWorkersHistogram->Output();

  Print("Kolikrát pro obsluhu nákladního vozidla není potřebný počet "
        "pracovníků: ");
  Print(notEnoughWorkers);
  Print("\n");

  Print("\n\n================== Experiment 3 ==================\n\n");

  fullTimeWorkerStat.Output();
  partTimeWorkerStat.Output();
  officeWorkerStat.Output();

  officeWorkersHistogram->Output();
  forkliftsHistogram->Output();
  forkliftStat.Output();

  Print("Počet nedokončených obsluh: ");
  Print(storageQueue.Length() + administrationQueue.Length());
  Print("\n");

  Print("\n\n================== Počty vygenerovaných vozidel "
        "==================\n\n");
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
  Print(loadingDeliveryTrucksGenerated + unloadingTrucksGenerated +
        loadingTrucksGenerated + unloadingDeliveryTrucksGenerated);
  Print("\n\n");

  Print("\n\n================== Čas v systému ==================\n\n");
  unloadingTruckSystemTime.Output();
  unloadingDeliveryTruckSystemTime.Output();
  loadingTruckSystemTime.Output();
  loadingDeliveryTruckSystemTime.Output();
  storeGoodsSystemTime.Output();

  Print("\n\n================== Ostatní ==================\n\n");

  officeWorkers->Output();

  forklifts->Output();

  storageQueue.Output();

  administrationQueue.Output();

  cerr << "Done" << endl;
  return 0;
}