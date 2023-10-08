/*
 * mixed-edabit-example.cpp
 * edabit is only used in BA mixed primitives, such as comparison, truncation, full b2a, full a2b,
 * all of which require a binary addition circuit, hence it is difficult to show a concrete computation 
 * example in this c++ backend. It is better to use the frontend compiler to test a use case.
 * 
 * If indeed necessary, can refer to `buffer_edabits_from_personal` in `Spdz2kPrep.hpp` for how it uses "GC/BitAdder.h".
 */

#define VERBOSE_DEBUG_PRINT

#include "Protocols/ProtocolSet.h"

#include "Machines/SPDZ.hpp"
#include "Machines/SPDZ2k.hpp"
#include "Machines/Semi2k.hpp"
// #include "Machines/Rep.hpp"
// #include "Machines/Rep4.hpp"
#include "Machines/Atlas.hpp"

template<class T>
void run(char** argv);

int main(int argc, char** argv)
{
    // need player number and number of players
    if (argc < 3)
    {
        cerr << "Usage: " << argv[0]
                << "<my number: 0/1/...> <total number of players> [protocol]"
                << endl;
        exit(1);
    }

    string protocol = "SPDZ2k";
    if (argc > 3)
        protocol = argv[3];

    if (protocol == "SPDZ2k")
        run<Spdz2kShare<64, 64>>(argv);
    else if (protocol == "Semi2k")
        run<Semi2kShare<64>>(argv);
    // else if (protocol == "Rep3")
    //     run<Rep3Share2<64>>(argv);
    // else if (protocol == "Rep4")
    //     run<Rep4Share2<64>>(argv);
    // else if (protocol == "Atlas")
    //     run<AtlasShare<gfp_<0, 2>>>(argv);
    else
    {
        cerr << "Unknown protocol: " << protocol << endl;
        exit(1);
    }
}

template<class T>
void run(char** argv)
{
    // reduce batch size
    // OnlineOptions::singleton.bucket_size = 5;
    OnlineOptions::singleton.batch_size = 1000;

    // set up networking on localhost
    int my_number = atoi(argv[1]);
    int n_parties = atoi(argv[2]);
    int port_base = 9999;
    Names N(my_number, n_parties, "localhost", port_base);
    // CryptoPlayer P(N);
    PlainPlayer P(N);

    // protocol setup (domain, MAC key if needed etc)
    MixedProtocolSetup<T> setup(P);

    // set of protocols (bit_input, multiplication, output)
    MixedProtocolSet<T> set(P, setup);
    auto& output = set.output;
    auto& bit_input = set.binary.input;
    auto& bit_protocol = set.binary.protocol;
    auto& bit_output = set.binary.output;
    auto& prep = set.preprocessing;

    int n_bits = 64;
    edabit<T> eb;
    print_total_comm(P, "Before gen");
    prep.get_edabit_no_count(false, n_bits, eb);
    print_total_comm(P, "After gen");

    bit_output.init_open(P, n_bits);
    for (int i = 0; i < n_bits; i++) {
        bit_output.prepare_open(eb.second[i]);
    }
    bit_output.exchange(P);
    typename T::clear x;
    for (int i = 0; i < n_bits; i++) {
        x += typename T::clear(bit_output.finalize_open()) << i;
    }
    cout << "edabit B compose: " << x << endl;

    output.init_open(P, 1);
    output.prepare_open(eb.first);
    output.exchange(P);

    cout << "edabit A: " << output.finalize_open() << endl;

    set.check();
}
