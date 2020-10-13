/*
 * paper-example.cpp
 *
 * Working example similar to Figure 2 in https://eprint.iacr.org/2020/521
 *
 */

#include "Math/gfp.hpp"
#include "Machines/SPDZ.hpp"

int main(int argc, char** argv)
{
    typedef Share<gfp> T;

    // need player number and number of players
    if (argc < 3)
    {
        cerr << "Usage: " << argv[0] << "<my number: 0/1/...> <total number of players>" << endl;
        exit(1);
    }

    // set up networking on localhost
    Names N;
    Server::start_networking(N, atoi(argv[1]), atoi(argv[2]), "localhost", 9999);
    CryptoPlayer P(N);

    // initialize fields
    gfp::init_default(128);
    gfp1::init_default(128, false);
    T::bit_type::mac_key_type::init_field();

    // must initialize MAC key for security of some protocols
    typename T::mac_key_type mac_key;
    T::read_or_generate_mac_key("", N, mac_key);
    typename T::bit_type::mac_key_type binary_mac_key;
    T::bit_type::part_type::read_or_generate_mac_key("", N, binary_mac_key);

    // global OT setup
    BaseMachine machine;
    machine.ot_setups.push_back({P});

    // keeps tracks of preprocessing usage (triples etc)
    DataPositions usage;
    usage.set_num_players(P.num_players());

    // binary MAC check setup
    GC::ShareThread<typename T::bit_type> thread(N,
            OnlineOptions::singleton, P, binary_mac_key, usage);

    // output protocol
    typename T::MAC_Check output(mac_key);

    // various preprocessing
    typename T::LivePrep preprocessing(0, usage);
    SubProcessor<T> processor(output, preprocessing, P);

    // input protocol
    typename T::Input input(processor);

    // multiplication protocol
    typename T::Protocol protocol(P);

    int n = 1000;
    vector<T> a(n), b(n);
    T c;
    typename T::clear result;

    input.reset_all(P);
    for (int i = 0; i < n; i++)
        input.add_from_all(i);
    input.exchange();
    for (int i = 0; i < n; i++)
    {
        a[i] = input.finalize(0);
        b[i] = input.finalize(1);
    }

    protocol.init_dotprod(&processor);
    for (int i = 0; i < n; i++)
        protocol.prepare_dotprod(a[i], b[i]);
    protocol.next_dotprod();
    protocol.exchange();
    c = protocol.finalize_dotprod(n);
    output.init_open(P);
    output.prepare_open(c);
    output.exchange(P);
    result = output.finalize_open();

    cout << "result: " << result << endl;
    output.Check(P);
}
