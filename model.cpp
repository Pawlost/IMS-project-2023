/*  FIT VUT - Modelování a simulace
**  SHO v logistice
**
**  Date: 2. 12. 2023
*/

#include "simlib.h"
#include <iostream>
#include <algorithm>

using namespace std;

#define SIMULATION_END_TIME 1000000

Store* fullTimeWorkers;
Store* partTimeWorkers;
Store* ramps;
Store* officialWorkers;
Store* forklifts;

Queue unloadingDeliveryTruckQueue("Fronta dodávek na vykládku");
Queue unloadingTruckQueue("Fronta kamionů na vykládku");
Queue loadingDeliveryTruckQueue("Fronta dodávek na nakládku");
Queue loadingTruckQueue("Fronta kamionů na nakládku");
Queue administrationQueue("Fronta na razítko");
Queue loadingQueue("Fronta na vykládku");

void showHelpMessage(string name) {
    cerr << "Správné použití:" <<
        endl;
}

bool findOptionString(char * start[], char * end[],
    const string & optionString) {
    return find(start, end, optionString) != end;
}

char * getOptionValue(char * start[], char * end[],
    const string & optionString,
        const string & optionStringLong) {
    /* Find option string and return option value */
    /* https://stackoverflow.com/questions/865668/parsing-command-line-arguments-in-c */
    char ** itr = find(start, end, optionString);
    if (itr != end && ++itr != end)
        return * itr;

    char ** itrLong = find(start, end, optionStringLong);
    if (itrLong != end && ++itrLong != end)
        return * itrLong;
    return 0;
}

void parseAllArguments(char ** start, char** end){
    if (findOptionString(start, end, "-h") || findOptionString(start, end, "--help")) {
        showHelpMessage(start[0]);
        exit(0);
    }
    char * outputFile = getOptionValue(start, end, "-f", "--file");
    if (outputFile){
        SetOutput(outputFile);
    }
}

int main(int argc, char * argv[]) {
    parseAllArguments(argv, argv + argc);

    Init(0, SIMULATION_END_TIME); // Initialize simulation
    Run(); // Run simulation

    return 0;
}