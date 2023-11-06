#define VERBOSE_DEBUG_PRINT

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


template<class T>
void test_buffer_triples(int argc, char** argv)
{
    OnlineOptions::singleton.batch_size = 10000;
    cout << "[zico] batch size: " << OnlineOptions::singleton.batch_size << endl;
    // set up networking on localhost
    int my_number = atoi(argv[1]);
    int n_parties = atoi(argv[2]);
    int port_base = 9999;
    Names N(my_number, n_parties, "localhost", port_base);
    PlainPlayer P(N);

    // protocol setup (domain, MAC key if needed etc)
    BinaryProtocolSetup<T> setup(P);

    // set of protocols (input, multiplication, output)
    BinaryProtocolSet<T> set(P, setup);
    auto& prep = set.prep;

    perf_log("Triples", P.total_comm().sent);
    prep.buffer_triples();
    perf_log("Triples", P.total_comm().sent);
    set.check();

    // auto comm_stats = P.total_comm();
    // size_t rounds = 0;
    // for (auto& x : comm_stats)
    //   rounds += x.second.rounds;
    // std::cerr << "Data sent = " << comm_stats.sent / 1e6 << " MB in ~" << rounds
    //     << " rounds (party " << my_number << std::endl;;
}

template<class T>
void test_buffer_inputs(int argc, char** argv)
{
    OnlineOptions::singleton.batch_size = 10000;
    // set up networking on localhost
    int my_number = atoi(argv[1]);
    int n_parties = atoi(argv[2]);
    int port_base = 9999;
    Names N(my_number, n_parties, "localhost", port_base);
    PlainPlayer P(N);

    // protocol setup (domain, MAC key if needed etc)
    BinaryProtocolSetup<T> setup(P);

    // set of protocols (input, multiplication, output)
    BinaryProtocolSet<T> set(P, setup);
    auto& prep = set.prep;

    perf_log("Inputs", P.total_comm().sent);
    prep.buffer_inputs(0);
    perf_log("Inputs", P.total_comm().sent);

    set.check();
}

template<class T>
void test_buffer_crypto2022_quintuples(int argc, char** argv)
{
    OnlineOptions::singleton.batch_size = 10000;
    cout << "[zico] batch size: " << OnlineOptions::singleton.batch_size << endl;
    // set up networking on localhost
    int my_number = atoi(argv[1]);
    int n_parties = atoi(argv[2]);
    int port_base = 9999;
    Names N(my_number, n_parties, "localhost", port_base);
    PlainPlayer P(N);

    // protocol setup (domain, MAC key if needed etc)
    BinaryProtocolSetup<T> setup(P);

    // set of protocols (input, multiplication, output)
    BinaryProtocolSet<T> set(P, setup);
    auto& prep = set.prep;

    perf_log("Crypto2022 quintuples", P.total_comm().sent);
    prep.buffer_crypto2022_quintuples();
    perf_log("Crypto2022 quintuples", P.total_comm().sent);
    set.check();

    // auto comm_stats = P.total_comm();
    // size_t rounds = 0;
    // for (auto& x : comm_stats)
    //   rounds += x.second.rounds;
    // std::cerr << "Data sent = " << comm_stats.sent / 1e6 << " MB in ~" << rounds
    //     << " rounds (party " << my_number << std::endl;;
}

int main(int argc, char** argv)
{
    string protocol = "coral";
    if (argc > 3)
        protocol = argv[3]; // coral, tiny, tinier, crypto2022

    string type = "inputs";
    if (argc > 4)
        type = argv[4]; // inputs, triples
    
    /* inputs */
    if (protocol == "coral" && type == "inputs") {
        test_buffer_inputs<GC::RmfeShare>(argc, argv);
    }
    else if (protocol == "crypto2022" && type == "inputs") {
        test_buffer_inputs<GC::RmfeShare>(argc, argv);
    }
    else if (protocol == "tinier" && type == "inputs") {
        test_buffer_inputs<GC::TinierSecret<gf2n_mac_key>>(argc, argv);
    }
    else if (protocol == "tiny" && type == "inputs") {
        test_buffer_inputs<GC::TinySecret<DEFAULT_SECURITY>>(argc, argv);
    }
    /* triples */
    else if (protocol == "coral" && type == "triples") {
        test_buffer_triples<GC::RmfeShare>(argc, argv);
    }
    else if (protocol == "crypto2022" && type == "triples") {
        test_buffer_crypto2022_quintuples<GC::RmfeShare>(argc, argv);
    }
    else if (protocol == "tinier" && type == "triples") {
        test_buffer_triples<GC::TinierSecret<gf2n_mac_key>>(argc, argv);
    }
    else if (protocol == "tiny" && type == "triples") {
        test_buffer_triples<GC::TinySecret<DEFAULT_SECURITY>>(argc, argv);
    }

}