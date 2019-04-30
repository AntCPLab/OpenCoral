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

    vector<Program<T> > progs;

    bool use_encryption;
    bool more_comm_less_comp;

    Machine();
    ~Machine();

    void load_schedule(string progname);
    void load_program(string threadname, string filename);

    template<class U>
    void reset(const U& program);
    template<class U, class V>
    void reset(const U& program, V& dynamic_memory);

    void start_timer() { timer[0].start(); }
    void stop_timer() { timer[0].stop(); }
    void reset_timer() { timer[0].reset(); }

    void run_tape(int thread_number, int tape_number, int arg);
    void join_tape(int thread_numer);

    void write_memory(int my_num);
};

} /* namespace GC */

#endif /* GC_MACHINE_H_ */
