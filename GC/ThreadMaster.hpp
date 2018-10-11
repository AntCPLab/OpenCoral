/*
 * ThreadMaster.cpp
 *
 */

#include "ThreadMaster.h"
#include "Program.h"

#include "ReplicatedSecret.h"
#include "Secret.h"
#include "Yao/YaoGarbleWire.h"
#include "Yao/YaoEvalWire.h"

namespace GC
{

template<class T>
ThreadMaster<T>* ThreadMaster<T>::singleton = 0;

template<class T>
ThreadMaster<T>& ThreadMaster<T>::s()
{
    if (singleton)
        return *singleton;
    else
        throw runtime_error("no singleton, maybe threads not supported");
}

template<class T>
ThreadMaster<T>::ThreadMaster() : P(0), machine(memory)
{
    if (singleton)
        throw runtime_error("there can only be one");
    singleton = this;
}

template<class T>
void ThreadMaster<T>::run_tape(int thread_number, int tape_number, int arg)
{
    threads.at(thread_number)->tape_schedule.push({tape_number, arg});
}

template<class T>
void ThreadMaster<T>::join_tape(int thread_number)
{
    threads.at(thread_number)->join_tape();
}

template<class T>
Thread<T>* ThreadMaster<T>::new_thread(int i)
{
    return new Thread<T>(i, machine, N);
}

template<class T>
void ThreadMaster<T>::run()
{
    P = new Player(N, 1 << 24);

    machine.load_schedule(progname);
    for (int i = 0; i < machine.nthreads; i++)
        threads.push_back(new_thread(i));
    for (auto thread : threads)
        thread->join_tape();

    Timer timer;
    timer.start();

    threads[0]->tape_schedule.push(0);

    for (auto thread : threads)
        thread->finish();

    // synchronize
    vector<octetStream> os(P->num_players());
    P->Broadcast_Receive(os);

    for (auto thread : threads)
        delete thread;

    delete P;

    cerr << "Time = " << timer.elapsed() << endl;
}


template class ThreadMaster<FakeSecret>;
template class ThreadMaster<ReplicatedSecret>;


} /* namespace GC */
