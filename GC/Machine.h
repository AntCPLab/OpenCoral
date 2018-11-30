/*
 * Machine.h
 *
 */

#ifndef GC_MACHINE_H_
#define GC_MACHINE_H_

#include <GC/FakeSecret.h>
#include "GC/Clear.h"
#include "GC/Memory.h"

#include "Processor/BaseMachine.h"

#include <vector>
using namespace std;

namespace GC
{

template <class T> class Program;

template <class T>
class Machine : public ::BaseMachine
{
public:
    Memory<T> MS;
    Memory<Clear> MC;
    Memory<Integer> MI;
    Memory<typename T::DynamicType>& MD;

    vector<Program<T> > progs;

    bool use_encryption;
    bool more_comm_less_comp;

    Machine(Memory<typename T::DynamicType>& MD);
    ~Machine();

    void load_schedule(string progname);
    void load_program(string threadname, string filename);

    void reset(const Program<T>& program);

    void start_timer() { timer[0].start(); }
    void stop_timer() { timer[0].stop(); }
    void reset_timer() { timer[0].reset(); }

    void run_tape(int thread_number, int tape_number, int arg);
    void join_tape(int thread_numer);
};

} /* namespace GC */

#endif /* GC_MACHINE_H_ */
