/*
 * ThreadMaster.cpp
 *
 */

#include "ThreadMaster.h"
#include "Program.h"

#include "instructions.h"

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
ThreadMaster<T>::ThreadMaster(OnlineOptions& opts) :
        P(0), opts(opts)
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
    return new Thread<T>(i, *this);
}

template<class T>
void ThreadMaster<T>::run()
{
    P = new PlainPlayer(N, 0xff << 24);

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

    post_run();

    NamedCommStats stats = P->comm_stats;
    ExecutionStats exe_stats;
    for (auto thread : threads)
    {
        stats += thread->P->comm_stats;
        exe_stats += thread->processor.stats;
        delete thread;
    }

    delete P;

    for (auto it : exe_stats)
        switch (it.first)
        {
#define X(NAME, CODE) case NAME: cerr << it.second << " " #NAME << endl; break;
        INSTRUCTIONS
#undef X
        }

    for (auto it = stats.begin(); it != stats.end(); it++)
        if (it->second.data > 0)
            cerr << it->first << " " << 1e-6 * it->second.data << " MB" << endl;

    cerr << "Time = " << timer.elapsed() << endl;
}

} /* namespace GC */
