
// [zico] need to update
#define NO_SECURITY_CHECK
#define VERBOSE_DEBUG_PRINT

#include "Machines/Coral.hpp"
#include "Protocols/ProtocolSet.h"
#include "Tools/performance.h"

template<class T>
void test_buffer_edabits(int argc, char** argv)
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
    MixedProtocolSetup<T> setup(P);

    // set of protocols (input, multiplication, output)
    MixedProtocolSet<T> set(P, setup);
    auto& prep = set.preprocessing;

    perf_log("loose edabit", P.total_comm().sent);
    prep.buffer_edabits(false, 64, 0);
    perf_log("loose edabit", P.total_comm().sent);

    perf_log("strict edabit", P.total_comm().sent);
    prep.buffer_edabits(true, 64, 0);
    perf_log("strict edabit", P.total_comm().sent);

    set.check();
}

int main(int argc, char** argv)
{
    cerr << "Usage: " << argv[0]
        << "<my number: 0/1/...> <total number of players> [protocol] [buffer_type]"
        << endl;

    string protocol = "coral";
    if (argc > 3)
        protocol = argv[3]; // coral, corallowgear, coralmascot, spdz2k, lowgear, mascot

    string type = "edabit";
    if (argc > 4)
        type = argv[4]; // edabit, dabit
    
    /* edabit */
    if (protocol == "coral" && type == "edabit") {
        test_buffer_edabits<CoralShare<64, 64>>(argc, argv);
    }
    else if (protocol == "corallowgear" && type == "edabit") {
        test_buffer_edabits<CoralLowGearShare<gfp_<0, 2>>>(argc, argv);
    }
    // else if (protocol == "coralmascot" && type == "inputs") {
    //     test_buffer_edabits<GC::TinierSecret<gf2n_mac_key>>(argc, argv);
    // }
    // else if (protocol == "spdz2k" && type == "inputs") {
    //     test_buffer_edabits<GC::TinySecret<DEFAULT_SECURITY>>(argc, argv);
    // }
    /* dabit */
    // else if (protocol == "coral" && type == "triples") {
    //     test_buffer_triples<GC::RmfeShare>(argc, argv);
    // }
    // else if (protocol == "crypto2022" && type == "triples") {
    //     test_buffer_crypto2022_quintuples<GC::RmfeShare>(argc, argv);
    // }
    // else if (protocol == "tinier" && type == "triples") {
    //     test_buffer_triples<GC::TinierSecret<gf2n_mac_key>>(argc, argv);
    // }
    // else if (protocol == "tiny" && type == "triples") {
    //     test_buffer_triples<GC::TinySecret<DEFAULT_SECURITY>>(argc, argv);
    // }

}
