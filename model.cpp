/*  FIT VUT - Modelování a simulace
**  SHO v logistice
**
**  Date: 2. 12. 2023
*/

#include "simlib.h"
#include <cstddef>
#include <iostream>
#include <algorithm>
#include <string>

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

void showHelp() {
    cerr << "Správné použití:" <<
        endl;

    exit(0);
}

void showHelp(const string shortName, const string longName, int minValue) {
    cerr << "Špatný argument" << shortName << " " << longName << ". Hodnota by měla začínat od " << minValue << ". ";
    showHelp();
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
    return NULL;
}

int getOptionNumber(char * start[], char * end[],
    const string & optionString,
        const string & optionStringLong, int minValue){
        char* option = getOptionValue(start, end, optionString, optionStringLong);

    if(!option || stoi(option) < minValue){
        showHelp(optionString, optionStringLong, minValue);
    }

    return stoi(option);
}

void parseAllArguments(char ** start, char** end){
    if (findOptionString(start, end, "-h") || findOptionString(start, end, "--help")) {
        showHelp();
    }
    char * outputFile = getOptionValue(start, end, "-f", "--file");
    if (outputFile){
        SetOutput(outputFile);
    }

    int option = getOptionNumber(start, end, "-z", "--ramps", 1);   
    ramps = new Store("Rampy", option);

    option = getOptionNumber(start, end, "-x", "--fullTimeWorkers", 2);
    fullTimeWorkers = new Store("Stálý pracovníci", option);

    option = getOptionNumber(start, end, "-y", "--partTimeWorkers", 4);
    partTimeWorkers = new Store("Brigádnící", option);

    option = getOptionNumber(start, end, "-v", "--officialWorkers", 1);
    officialWorkers = new Store("Úředníci", option);

    option = getOptionNumber(start, end, "-w", "--forklifts", 1);
    forklifts = new Store("Vysokozdvižné vozíky", option);
}

int main(int argc, char * argv[]) {
    parseAllArguments(argv, argv + argc);

    Init(0, SIMULATION_END_TIME); // Initialize simulation

    Run(); // Run simulation

    cerr << "Finished simulation, printing results" << endl;
    
    ramps->Output();
    fullTimeWorkers->Output();;
    partTimeWorkers->Output();;
    officialWorkers->Output();;
    forklifts->Output();

    cerr << "Done" << endl;
    return 0;
}