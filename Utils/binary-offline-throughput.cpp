/*
 * Reference: ot-offline.cpp
 *
 */

#define VERBOSE_DEBUG_PRINT

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

string buffer_type = "inputs";

class PrepThread {
public:
    long generated;
    virtual void generate() = 0;

    virtual ~PrepThread() {}
};

template<class T>
class BinaryPrepThread : public PrepThread
{
public:
    Player* P;

    BinaryPrepThread(const Names& names, int thread_num) 
        : P(new PlainPlayer(names, "thread" + to_string(thread_num))) {
    }

    void generate() {
        T::Protocol::setup(*P);
        BinaryProtocolThreadInit<T>::setup(*P);

        typename T::LivePrep* prep = dynamic_cast<typename T::LivePrep*>(&GC::ShareThread<T>::s().DataF);
        if (buffer_type == "inputs")
        {
            prep->buffer_inputs(0);
            generated = prep->get_inputs_size(0) * T::default_length;
            cout << "Generated: " << generated << " inputs" << endl;
        }
        else if (buffer_type == "triples") {
            prep->buffer_triples();
            generated = prep->get_triples_size() * T::default_length;
            cout << "Generated: " << generated << " triples" << endl;
        }
        else if (buffer_type == "crypto2022"){
            prep->buffer_crypto2022_quintuples();
            generated = prep->get_triples_size() * T::default_length;
            cout << "Generated: " << generated << " triples" << endl;
        }
    }

    virtual ~BinaryPrepThread() {
        delete P;
    }
};

void* run_ngenerator_thread(void* ptr)
{
    ((PrepThread*)ptr)->generate();
    return 0;
}

class BinaryOfflineMachine : public OfflineMachineBase
{
    Names N[2];
    int nConnections;
    Player* player;
    NetworkOptionsWithNumber network_opts;
    string prot;

public:
    BinaryOfflineMachine(int argc, const char** argv);

    template<class T>
    BinaryPrepThread<T>* new_generator(int i);

    void run();
};


BinaryOfflineMachine::BinaryOfflineMachine(int argc, const char** argv) :
        nConnections(1), player(0),
        network_opts(opt, argc, argv, 2, true)
{

    opt.add(
        "coral", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Protocol to run (coral, tinier, tiny, crypto2022)", // Help description.
        "-prot", // Flag token.
        "--prot" // Flag token.
    );

    opt.add(
        "inputs", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Buffer type (inputs, triples, crypto2022)", // Help description.
        "-type", // Flag token.
        "--type" // Flag token.
    );

    parse_options(argc, argv);

    if (opt.isSet("--prot"))
    {
        opt.get("--prot")->getString(prot);
    }
    else 
        prot = "coral";
    
    if (opt.isSet("--type"))
    {
        opt.get("--type")->getString(buffer_type);
    }
    else 
        buffer_type = "inputs";
}

template<class T>
BinaryPrepThread<T>* BinaryOfflineMachine::new_generator(int i)
{
    return new BinaryPrepThread<T>(N[i % nConnections], i);
}

void BinaryOfflineMachine::run()
{
    cout << "my_num: " << my_num << ", nthreads: " << nthreads << endl;
    network_opts.start_networking(N[0], my_num);
    nConnections = 1;

    PlainPlayer P(N[0], "base");
    player = &P;

    if (prot == "coral") {
        BinaryProtocolThreadInit<GC::RmfeShare>::setup(P);
    }
    else if (prot == "tinier") {
        BinaryProtocolThreadInit<GC::TinierSecret<gf2n_mac_key>>::setup(P);
    }
    else if (prot == "tiny") {
        BinaryProtocolThreadInit<GC::TinySecret<DEFAULT_SECURITY>>::setup(P);
    }
    else {
        throw std::runtime_error("unknown protocol");
    }

    vector<PrepThread*> generators(nthreads);
    vector<pthread_t> threads(nthreads);

    for (int i = 0; i < nthreads; i++)
    {
        if (prot == "coral")
            generators[i] = new_generator<GC::RmfeShare>(i);
        else if (prot == "tinier")
            generators[i] = new_generator<GC::TinierSecret<gf2n_mac_key>>(i);
        else if (prot == "tiny")
            generators[i] = new_generator<GC::TinySecret<DEFAULT_SECURITY>>(i);
        else {
           throw std::runtime_error("unknown protocol");
        }
    }
    cout <<"Setup generators\n";
    for (int i = 0; i < nthreads; i++)
    {
        // lock before starting thread to avoid race condition
        // generators[i]->lock();
        pthread_create(&threads[i], 0, run_ngenerator_thread, generators[i]);
    }

    // wait for initialization, then start clock and computation
    // for (int i = 0; i < nthreads; i++)
    //     generators[i]->wait();
    cout << "Starting computation" << endl;
    struct timeval start, stop;
    gettimeofday(&start, 0);
    // for (int i = 0; i < nthreads; i++)
    // {
    //     generators[i]->signal();
    //     generators[i]->unlock();
    // }

    // wait for threads to finish
    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(threads[i],NULL);
        cout << "thread " << i+1 << " finished\n" << flush;
    }
    long generated = 0;
    for (int i = 0; i < nthreads; i++)
        generated += generators[i]->generated;

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
    cout << "Throughput: " << generated / time << endl;

    for (size_t i = 0; i < generators.size(); i++)
        delete generators[i];
}


int main(int argc, const char** argv)
{
    BinaryOfflineMachine(argc, argv).run();
}
