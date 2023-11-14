/*
 * Reference: binary-offline-throughput.cpp
 *
 */

#define VERBOSE_DEBUG_PRINT
#define NO_SECURITY_CHECK

#include "Machines/Coral.hpp"
#include "Protocols/ProtocolSet.h"

#include "OT/NPartyTripleGenerator.h"
#include "OT/TripleMachine.h"

#include "Machines/Rmfe.hpp"

#include "Tools/debug.hpp"
#include "GC/TinierSecret.h"
#include "GC/PostSacriSecret.h"
#include "GC/CcdSecret.h"
#include "GC/MaliciousCcdSecret.h"
#include "GC/AtlasSecret.h"
#include "GC/TinyMC.h"
#include "GC/VectorInput.h"
#include "GC/PostSacriBin.h"
#include "Protocols/ProtocolSet.h"

#include "GC/ShareSecret.hpp"
#include "GC/CcdPrep.hpp"
#include "GC/TinierSharePrep.hpp"
#include "GC/RepPrep.hpp"
#include "GC/Secret.hpp"
#include "GC/TinyPrep.hpp"
#include "GC/ThreadMaster.hpp"
#include "GC/SemiSecret.hpp"
#include "Protocols/Atlas.hpp"
#include "Protocols/MaliciousRepPrep.hpp"
#include "Protocols/Share.hpp"
#include "Protocols/MaliciousRepMC.hpp"
#include "Protocols/Shamir.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Machines/ShamirMachine.hpp"
#include "Machines/Rep4.hpp"
#include "Machines/Rep.hpp"

#include "Tools/performance.h"

string buffer_type = "looseedabit";
int n_bits = 64;

class PrepThread {
public:
    pthread_mutex_t mutex;
    pthread_cond_t ready;

    PrepThread() {
        pthread_mutex_init(&mutex, 0);
        pthread_cond_init(&ready, 0);
    }

    long comm;
    long generated;
    virtual void generate() = 0;

    void lock() {
        pthread_mutex_lock(&mutex);
    }

    void unlock() {
        pthread_mutex_unlock(&mutex);
    }

    void signal() {
        pthread_cond_signal(&ready);
    }
    void wait() {
        pthread_cond_wait(&ready, &mutex);
    }

    virtual ~PrepThread() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&ready);
    }
};

template<class T>
class MixedPrepThread : public PrepThread
{
public:
    Player* P;
    MixedProtocolSetup<T> * setup;

    MixedPrepThread(const Names& names, int thread_num, MixedProtocolSetup<T> * setup) 
        : P(new PlainPlayer(names, "thread" + to_string(thread_num))), setup(setup) {
    }

    void generate() {
        T::Protocol::setup(*P);
        T::bit_type::Protocol::setup(*P);
        // BinaryProtocolThreadInit<T::bit_type>::setup(*P);

        MixedProtocolSet<T> set(*P, *setup);
        auto& prep = set.preprocessing;

        this->lock();
        this->signal();
        this->wait();

        auto perf = ThreadPerformance(buffer_type, P->total_comm().sent);
        if (buffer_type == "looseedabit")
        {
            for (int i = 0; i < 2; i++)
                prep.buffer_edabits(false, n_bits, 0);
            generated = prep.get_edabit_size(false, n_bits);
            cout << "Generated: " << generated << " loose edabits" << endl;
        }
        else if (buffer_type == "strictedabit")
        {
            for (int i = 0; i < 2; i++)
                prep.buffer_edabits(true, n_bits, 0);
            generated = prep.get_edabit_size(true, n_bits);
            cout << "Generated: " << generated << " strict edabits" << endl;
        }
        perf.stop(P->total_comm().sent);
        comm = perf.total_comm;
    }

    virtual ~MixedPrepThread() {
        delete P;
    }
};

void* run_ngenerator_thread(void* ptr)
{
    ((PrepThread*)ptr)->generate();
    return 0;
}

class MixedOfflineMachine : public OfflineMachineBase
{
    Names N[2];
    int nConnections;
    Player* player;
    NetworkOptionsWithNumber network_opts;
    string prot;

    int argc;
    const char** argv;

public:
    MixedOfflineMachine(int argc, const char** argv);

    template<class T>
    MixedPrepThread<T>* new_generator(int i, MixedProtocolSetup<T>* setup);

    void run();
};


MixedOfflineMachine::MixedOfflineMachine(int argc, const char** argv) :
        nConnections(1), player(0),
        network_opts(opt, argc, argv, 2, true),
        argc(argc), argv(argv)
{

    opt.add(
        "coral", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Protocol to run (mascot, spdz2k, lowgear, coralmascot, coral, corallowgear)", // Help description.
        "-prot", // Flag token.
        "--prot" // Flag token.
    );

    opt.add(
        "inputs", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Buffer type (looseedabit, strictedabit)", // Help description.
        "-type", // Flag token.
        "--type" // Flag token.
    );

    opt.add(
        "64", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Bit length of edabit", // Help description.
        "-nbits", // Flag token.
        "--nbits" // Flag token.
    );

    parse_options(argc, argv);

    if (opt.isSet("--type"))
    {
        opt.get("--type")->getString(buffer_type);
    }
    else 
        buffer_type = "looseedabit";

    if (opt.isSet("--nbits")) {
        opt.get("--nbits")->getInt(n_bits);
    }
    else n_bits = 64;

    if (opt.isSet("--prot"))
    {
        opt.get("--prot")->getString(prot);
    }
    else 
        prot = "coral";
}

template<class T>
MixedPrepThread<T>* MixedOfflineMachine::new_generator(int i, MixedProtocolSetup<T>* setup)
{
    return new MixedPrepThread<T>(N[i % nConnections], i, setup);
}

void MixedOfflineMachine::run()
{
    cout << "my_num: " << my_num << ", nthreads: " << nthreads << ", batch size: " << OnlineOptions::singleton.batch_size << endl;
    network_opts.start_networking(N[0], my_num);
    nConnections = 1;

    PlainPlayer P(N[0], "base");
    player = &P;

    // bit length of prime
    const int prime_length = 128;

    // compute number of 64-bit words needed
    const int n_limbs = (prime_length + 63) / 64;

    if (prot == "coral") {
        MixedProtocolThreadInit<CoralShare<64, 64>>::setup(P, prime_length, get_prep_sub_dir<CoralShare<64, 64>>(P.num_players()), true);
    }
    else if (prot == "coralmascot") {
        MixedProtocolThreadInit<CoralMascotShare<gfp_<0, n_limbs>>>::setup(P, prime_length, get_prep_sub_dir<CoralMascotShare<gfp_<0, n_limbs>>>(P.num_players()), true);
    }
    else if (prot == "corallowgear") {
        ez::ezOptionParser opt;
        CowGearOptions::singleton = CowGearOptions(opt, argc, argv, false);
        MixedProtocolThreadInit<CoralLowGearShare<gfp_<0, n_limbs>>>::setup(P, prime_length, get_prep_sub_dir<CoralLowGearShare<gfp_<0, n_limbs>>>(P.num_players()), true);
    }
    else if (prot == "spdz2k") {
        MixedProtocolThreadInit<Spdz2kShare<64, 64>>::setup(P, prime_length, get_prep_sub_dir<Spdz2kShare<64, 64>>(P.num_players()), true);
    }
    else if (prot == "mascot") {
        MixedProtocolThreadInit<Share<gfp_<0, n_limbs>>>::setup(P, prime_length, get_prep_sub_dir<Share<gfp_<0, n_limbs>>>(P.num_players()), true);
    }
    else if (prot == "lowgear") {
        ez::ezOptionParser opt;
        CowGearOptions::singleton = CowGearOptions(opt, argc, argv, false);
        MixedProtocolThreadInit<LowGearShare<gfp_<0, n_limbs>>>::setup(P, prime_length, get_prep_sub_dir<LowGearShare<gfp_<0, n_limbs>>>(P.num_players()), true);
    }
    else {
        throw std::runtime_error("unknown protocol");
    }

    vector<PrepThread*> generators(nthreads);
    vector<pthread_t> threads(nthreads);

    for (int i = 0; i < nthreads; i++)
    {
        if (prot == "coral")
            generators[i] = new_generator<CoralShare<64, 64>>(i, MixedProtocolThreadInit<CoralShare<64, 64>>::global_setup.get());
        else if (prot == "coralmascot")
            generators[i] = new_generator<CoralMascotShare<gfp_<0, n_limbs>>>(i, MixedProtocolThreadInit<CoralMascotShare<gfp_<0, n_limbs>>>::global_setup.get());
        else if (prot == "corallowgear")
            generators[i] = new_generator<CoralLowGearShare<gfp_<0, n_limbs>>>(i, MixedProtocolThreadInit<CoralLowGearShare<gfp_<0, n_limbs>>>::global_setup.get());
        else if (prot == "spdz2k")
            generators[i] = new_generator<Spdz2kShare<64, 64>>(i, MixedProtocolThreadInit<Spdz2kShare<64, 64>>::global_setup.get());
        else if (prot == "mascot")
            generators[i] = new_generator<Share<gfp_<0, n_limbs>>>(i, MixedProtocolThreadInit<Share<gfp_<0, n_limbs>>>::global_setup.get());
        else if (prot == "lowgear")
            generators[i] = new_generator<LowGearShare<gfp_<0, n_limbs>>>(i, MixedProtocolThreadInit<LowGearShare<gfp_<0, n_limbs>>>::global_setup.get());
        else {
           throw std::runtime_error("unknown protocol");
        }
    }
    cout <<"Setup generators\n";
    for (int i = 0; i < nthreads; i++)
    {
        // lock before starting thread to avoid race condition
        generators[i]->lock();
        pthread_create(&threads[i], 0, run_ngenerator_thread, generators[i]);
    }

    // wait for initialization, then start clock and computation
    for (int i = 0; i < nthreads; i++) {
        generators[i]->wait();
    }
    cout << "Starting computation" << endl;
    struct timeval start, stop;
    gettimeofday(&start, 0);
    for (int i = 0; i < nthreads; i++)
    {
        generators[i]->signal();
        generators[i]->unlock();
    }

    // wait for threads to finish
    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(threads[i],NULL);
        cout << "thread " << i+1 << " finished\n" << flush;
    }
    long generated = 0;
    long comm = 0;
    for (int i = 0; i < nthreads; i++) {
        generated += generators[i]->generated;
        comm += generators[i]->comm;
    }

    // map<string,Timer>& timers = generators[0]->timers;
    // for (map<string,Timer>::iterator it = timers.begin(); it != timers.end(); it++)
    // {
    //     double sum = 0;
    //     for (size_t i = 0; i < generators.size(); i++)
    //         sum += generators[i]->timers[it->first].elapsed();
    //     cout << it->first << " on average took time "
    //             << sum / generators.size() << endl;
    // }

    gettimeofday(&stop, 0);
    double time = timeval_diff_in_seconds(&start, &stop);
    cout << "Time: " << time << endl;
    cout << "Comm / per 1000 ops: " << comm * 1.0 / 1e6 / generated * 1000  << " MB" << endl;
    cout << "Throughput: " << generated / time << endl;

    for (size_t i = 0; i < generators.size(); i++)
        delete generators[i];
}


int main(int argc, const char** argv)
{
    MixedOfflineMachine(argc, argv).run();
}
