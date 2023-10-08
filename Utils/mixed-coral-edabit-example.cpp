/*
 * mixed-edabit-example.cpp
 * edabit is only used in BA mixed primitives, such as comparison, truncation, full b2a, full a2b,
 * all of which require a binary addition circuit, hence it is difficult to show a concrete computation 
 * example in this c++ backend. It is better to use the frontend compiler to test a use case.
 * 
 * If indeed necessary, can refer to `buffer_edabits_from_personal` in `Spdz2kPrep.hpp` for how it uses "GC/BitAdder.h".
 */

// [zico] need to update
#define NO_SECURITY_CHECK
#define VERBOSE_DEBUG_PRINT

#include "Protocols/ProtocolSet.h"
#include "Machines/Coral.hpp"
#include "Tools/performance.h"

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

    string protocol = "Coral";
    if (argc > 3)
        protocol = argv[3];

    if (protocol == "Coral")
        run<CoralShare<64, 64>>(argv);
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

    cout << "Online options: batch_size = " << OnlineOptions::singleton.batch_size << endl;

    int dl = T::bit_type::default_length;
    int n_bits = 16;

    cout << endl << "LOOSE: " << endl;
    print_total_comm(P, "Before gen");
    edabitpack<T> eb = prep.get_edabitpack_no_count(false, 64);
    print_total_comm(P, "After gen");
    reveal(&P, eb, "loose eb");

    // cout << endl << "STRICT: " << endl;
    // edabitpack<T> eb2 = prep.get_edabitpack_no_count(true, 64);
    // reveal(&P, eb2, "strict eb");

    set.check();
    print_profiling();
}
