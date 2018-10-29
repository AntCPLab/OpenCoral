/*
 * Thread.cpp
 *
 */

#include "Thread.h"
#include "Program.h"

#include "ReplicatedSecret.h"
#include "Secret.h"

namespace GC
{

template<class T>
void* Thread<T>::run_thread(void* thread)
{
    ((Thread<T>*)thread)->run();
    return 0;
}

template<class T>
Thread<T>::Thread(int thread_num, Machine<T>& machine, Names& N) :
        machine(machine), processor(machine), protocol(0), N(N), P(0),
        thread_num(thread_num)
{
    pthread_create(&thread, 0, run_thread, this);
}

template<class T>
Thread<T>::~Thread()
{
    if (P)
        delete P;
    if (protocol)
        delete protocol;
}

template<class T>
void Thread<T>::run()
{
    if (singleton)
         throw runtime_error("there can only be one");
    singleton = this;
    secure_prng.ReSeed();
    P = new Player(N, thread_num << 16);
    protocol = new typename T::Protocol(*P);
    string input_file = "Player-Data/Input-P" + to_string(N.my_num()) + "-" + to_string(thread_num);
    processor.open_input_file(input_file);
    done.push(0);
    pre_run();

    ScheduleItem item;
    while (tape_schedule.pop_dont_stop(item))
    {
        processor.reset(machine.progs.at(item.tape), item.arg);
        run(machine.progs[item.tape]);
        done.push(0);
    }

    post_run();
    MC.Check(*P);
}

template<class T>
void Thread<T>::run(Program<T>& program)
{
    while (program.execute(processor) != DONE_BREAK)
        ;
}

template<class T>
void Thread<T>::join_tape()
{
    int _;
    done.pop(_);
}

template<class T>
void Thread<T>::finish()
{
    tape_schedule.stop();
    pthread_join(thread, 0);
}

} /* namespace GC */
