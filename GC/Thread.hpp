/*
 * Thread.cpp
 *
 */

#include "Thread.h"
#include "Program.h"

#include "Networking/CryptoPlayer.h"

namespace GC
{

template<class T>
void* Thread<T>::run_thread(void* thread)
{
    ((Thread<T>*)thread)->run();
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    OPENSSL_thread_stop();
#endif
    return 0;
}

template<class T>
Thread<T>::Thread(int thread_num, ThreadMaster<T>& master) :
        master(master), machine(master.machine), processor(machine),
        protocol(0), N(master.N), P(0),
        thread_num(thread_num)
{
    pthread_create(&thread, 0, run_thread, this);
}

template<class T>
Thread<T>::~Thread()
{
    if (MC)
        delete MC;
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
    if (machine.use_encryption)
        P = new CryptoPlayer(N, thread_num << 16);
    else
        P = new PlainPlayer(N, thread_num << 16);
    protocol = new typename T::Protocol(*P);
    MC = this->new_mc();
    processor.open_input_file(N.my_num(), thread_num);
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
    MC->Check(*P);
}

template<class T>
void Thread<T>::run(Program<T>& program)
{
    while (program.execute(processor, master.memory) != DONE_BREAK)
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


template<class T>
int GC::Thread<T>::n_interactive_inputs_from_me(InputArgList& args)
{
    int res = 0;
    if (thread_num == 0 and master.opts.interactive)
        res = args.n_inputs_from(P->my_num());
    if (res > 0)
        cout << "Please enter " << res << " numbers:" << endl;
    return res;
}

} /* namespace GC */
