
// [zico] need to update
#define NO_SECURITY_CHECK
#define VERBOSE_DEBUG_PRINT

#include "Machines/Coral.hpp"
#include "Protocols/ProtocolSet.h"
#include "Tools/performance.h"

template<class T>
void test_buffer_edabits(int argc, const char** argv, bool strict, int prime_length = 0)
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
    // Need to call this first so that a MAC key is generated and written to a directory
    // And Spdz2k A and B sharings will read the same key
    // read_generate_write_mac_key<T>(P);
    MixedProtocolSetup<T> setup(P, prime_length, get_prep_sub_dir<T>(P.num_players()), true);

    // set of protocols (input, multiplication, output)
    MixedProtocolSet<T> set(P, setup);
    auto& prep = set.preprocessing;

    auto start = perf_log((strict? "strict" : "loose") + string(" edabit"), P.total_comm().sent);
    for (int i = 0; i < 2; i++)
        prep.buffer_edabits(strict, 64, 0);
    auto diff = perf_log((strict? "strict" : "loose") + string(" edabit"), P.total_comm().sent);
    cout << "[Time/1000 ops] " << diff.first.count() * 1.0 / 1e6 / prep.get_edabit_size(strict, 64) * 1000 << " ms" << endl;
    cout << "[Comm/1000 ops] " << diff.second * 1.0 / 1e6 / prep.get_edabit_size(strict, 64) * 1000 << " MB" << endl;

    set.check();
}

template<class T>
void test_buffer_dabits(int argc, const char** argv, int prime_length = 0)
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
    // Need to call this first so that a MAC key is generated and written to a directory
    // And Spdz2k A and B sharings will read the same key
    // read_generate_write_mac_key<T>(P);
    MixedProtocolSetup<T> setup(P, prime_length, get_prep_sub_dir<T>(P.num_players()), true);

    // set of protocols (input, multiplication, output)
    MixedProtocolSet<T> set(P, setup);
    auto& prep = set.preprocessing;

    auto start = perf_log(string("dabit"), P.total_comm().sent);
    for (int i = 0; i < 2; i++)
        prep.buffer_dabits(0);
    auto diff = perf_log(string("dabit"), P.total_comm().sent);
    cout << "[Time/1000 ops] " << diff.first.count() * 1.0 / 1e6 / prep.get_dabit_size() * 1000 << " ms" << endl;
    cout << "[Comm/1000 ops] " << diff.second * 1.0 / 1e6 / prep.get_dabit_size() * 1000 << " MB" << endl;

    set.check();
}


int main(int argc, const char** argv)
{
    cerr << "Usage: " << argv[0]
        << "<my number: 0/1/...> <total number of players> [protocol] [buffer_type] [strict|loose]"
        << endl;

    // bit length of prime
    const int prime_length = 128;

    // compute number of 64-bit words needed
    const int n_limbs = (prime_length + 63) / 64;

    string protocol = "coral";
    if (argc > 3)
        protocol = argv[3]; // coral, corallowgear, coralmascot, spdz2k, lowgear, mascot

    string type = "edabit";
    if (argc > 4)
        type = argv[4]; // edabit, dabit

    bool strict = false;
    if (argc > 5)
        strict = (string(argv[5]) == string("strict"));
    
    /* edabit */
    if (protocol == "coral" && type == "edabit") {
        test_buffer_edabits<CoralShare<64, 64>>(argc, argv, strict);
    }
    else if (protocol == "corallowgear" && type == "edabit") {
        ez::ezOptionParser opt;
        CowGearOptions::singleton = CowGearOptions(opt, argc, argv, false);
        test_buffer_edabits<CoralLowGearShare<gfp_<0, n_limbs>>>(argc, argv, strict, prime_length);
    }
    else if (protocol == "coralmascot" && type == "edabit") {
        test_buffer_edabits<CoralMascotShare<gfp_<0, n_limbs>>>(argc, argv, strict, prime_length);
    }
    else if (protocol == "spdz2k" && type == "edabit") {
        test_buffer_edabits<Spdz2kShare<64, 64>>(argc, argv, strict);
    }
    else if (protocol == "mascot" && type == "edabit") {
        test_buffer_edabits<Share<gfp_<0, n_limbs>>>(argc, argv, strict, prime_length);
    }
    else if (protocol == "lowgear" && type == "edabit") {
        ez::ezOptionParser opt;
        CowGearOptions::singleton = CowGearOptions(opt, argc, argv, false);
        test_buffer_edabits<LowGearShare<gfp_<0, n_limbs>>>(argc, argv, strict, prime_length);
    }
    /* dabit */
    else if (protocol == "coral" && type == "dabit") {
        test_buffer_dabits<CoralShare<64, 64>>(argc, argv);
    }
    else if (protocol == "corallowgear" && type == "dabit") {
        ez::ezOptionParser opt;
        CowGearOptions::singleton = CowGearOptions(opt, argc, argv, false);
        test_buffer_dabits<CoralLowGearShare<gfp_<0, n_limbs>>>(argc, argv, prime_length);
    }
    else if (protocol == "coralmascot" && type == "dabit") {
        test_buffer_dabits<CoralMascotShare<gfp_<0, n_limbs>>>(argc, argv, prime_length);
    }
    else if (protocol == "spdz2k" && type == "dabit") {
        test_buffer_dabits<Spdz2kShare<64, 64>>(argc, argv);
    }
    else if (protocol == "mascot" && type == "dabit") {
        test_buffer_dabits<Share<gfp_<0, n_limbs>>>(argc, argv, prime_length);
    }
    else if (protocol == "lowgear" && type == "dabit") {
        ez::ezOptionParser opt;
        CowGearOptions::singleton = CowGearOptions(opt, argc, argv, false);
        test_buffer_dabits<LowGearShare<gfp_<0, n_limbs>>>(argc, argv, prime_length);
    }
}
