
#define NO_MIXED_CIRCUITS

#include "Processor/Data_Files.h"
#include "Machines/SPDZ2k.hpp"
#include "Math/gf2n.h"
#include "Protocols/Spdz2kPrep.h"
#include "Processor/RingOptions.h"
#include "Processor/RingMachine.hpp"
#include "Processor/OnlineMachineExt.h"
#include "Protocols/ProtocolSetup.h"
#include "Protocols/ProtocolSet.h"


typedef Spdz2kShare<64, 64> sint;

void test_dotprod(int argc, char** argv) {
    // set up networking on localhost
    int my_number = atoi(argv[1]);
    int n_parties = atoi(argv[2]);
    int port_base = 9999;
    Names N(my_number, n_parties, "localhost", port_base);
    PlainPlayer P(N);

    // protocol setup (domain, MAC key if needed etc)
    ProtocolSetup<sint> setup(P);
    setup.set_batch_size(1000); // Make batch size match the inner product vector length so we have a tight benchmark.

    // set of protocols (input, multiplication, output)
    ProtocolSet<sint> set(P, setup);
    auto& input = set.input;
    auto& protocol = set.protocol;
    auto& output = set.output;

    int n = 1000;
    vector<sint> a(n), b(n);
    sint c;
    typename sint::clear result;

    input.reset_all(P);
    for (int i = 0; i < n; i++)
        input.add_from_all(i);
    input.exchange();
    for (int i = 0; i < n; i++)
    {
        a[i] = input.finalize(0);
        b[i] = input.finalize(1);
    }

    protocol.init_dotprod();
    for (int i = 0; i < n; i++)
        protocol.prepare_dotprod(a[i], b[i]);
    protocol.next_dotprod();
    protocol.exchange();
    c = protocol.finalize_dotprod(n);

    // protocol check before revealing results
    set.check();

    output.init_open(P);
    output.prepare_open(c);
    output.exchange(P);
    result = output.finalize_open();

    cout << "result: " << result << endl;

    // result check after opening
    set.check();

    auto comm_stats = P.total_comm();
    size_t rounds = 0;
    for (auto& x : comm_stats)
      rounds += x.second.rounds;
    std::cerr << "Data sent = " << comm_stats.sent / 1e6 << " MB in ~" << rounds
        << " rounds (party " << my_number << std::endl;;
}

void test_buffer_inputs(int argc, char** argv) {
    // set up networking on localhost
    int my_number = atoi(argv[1]);
    int n_parties = atoi(argv[2]);
    int port_base = 9999;
    Names N(my_number, n_parties, "localhost", port_base);
    PlainPlayer P(N);

    // protocol setup (domain, MAC key if needed etc)
    ProtocolSetup<sint> setup(P);
    // The batch size is set for Triples. Input batch size will be set to 10*Triples_batch_size, as shown in OT/NPartyTripleGenerator.hpp line 219.
    // So, should pay attention to this difference when benchmarking cost of Input vs Triple.
    setup.set_batch_size(1000);

    // set of protocols (input, multiplication, output)
    ProtocolSet<sint> set(P, setup);
    auto& input = set.input;
    auto& prep = set.preprocessing;

    print_total_comm(P, "Before Inputs");
    input.reset_all(P);
    input.add_from_all(0);
    print_total_comm(P, "After Inputs");

    // auto comm_stats = P.total_comm();
    // size_t rounds = 0;
    // for (auto& x : comm_stats)
    //   rounds += x.second.rounds;
    // std::cerr << "Data sent = " << comm_stats.sent / 1e6 << " MB in ~" << rounds
    //     << " rounds (party " << my_number << std::endl;;
}

template <class T>
void test_buffer_triples(int argc, char** argv, int prime_length = 0) {

    // set up networking on localhost
    int my_number = atoi(argv[1]);
    int n_parties = atoi(argv[2]);
    int port_base = 9999;
    Names N(my_number, n_parties, "localhost", port_base);
    PlainPlayer P(N);

    // protocol setup (domain, MAC key if needed etc)
    ProtocolSetup<T> setup(P, prime_length);
    setup.set_batch_size(10000);

    // set of protocols (input, multiplication, output)
    ProtocolSet<T> set(P, setup);
    auto& prep = set.preprocessing;

    prep.buffer_triples();

    auto comm_stats = P.total_comm();
    size_t rounds = 0;
    for (auto& x : comm_stats)
      rounds += x.second.rounds;
    std::cerr << "Data sent = " << comm_stats.sent / 1e6 << " MB in ~" << rounds
        << " rounds (party " << my_number << std::endl;;
}

int main(int argc, char** argv) {
    // test_dotprod(argc, argv);
    test_buffer_inputs(argc, argv);
    test_buffer_triples<sint>(argc, argv);
    {// Mascot
        // bit length of prime
        const int prime_length = 64;
        // compute number of 64-bit words needed
        const int n_limbs = (prime_length + 63) / 64;
        test_buffer_triples<Share<gfp_<0, n_limbs>>>(argc, argv, prime_length);
    }
}