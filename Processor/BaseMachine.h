/*
 * BaseMachine.h
 *
 */

#ifndef PROCESSOR_BASEMACHINE_H_
#define PROCESSOR_BASEMACHINE_H_

#include "Tools/time-func.h"
#include "OT/OTTripleSetup.h"

#include <map>
#include <fstream>
using namespace std;

class BaseMachine
{
protected:
    static BaseMachine* singleton;

    std::map<int,Timer> timer;

    ifstream inpf;

    void print_timers();

    virtual void load_program(string threadname, string filename);

public:
    string progname;
    int nthreads;

    vector<OTTripleSetup> ot_setups;

    static BaseMachine& s();

    static string memory_filename(string type_short, int my_number);

    BaseMachine();
    virtual ~BaseMachine() {}

    void load_schedule(string progname);
    void print_compiler();

    void time();
    void start(int n);
    void stop(int n);

    virtual void reqbl(int n) { (void)n; throw runtime_error("not defined"); }
};

#endif /* PROCESSOR_BASEMACHINE_H_ */
