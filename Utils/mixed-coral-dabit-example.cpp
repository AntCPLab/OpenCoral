/*
 * mixed-example.cpp
 *
 */

// [zico] need to update
#define NO_SECURITY_CHECK

#include "Protocols/ProtocolSet.h"

#include "Machines/Coral.hpp"
#include "Tools/debug.h"


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
    OnlineOptions::singleton.bucket_size = 5;
    OnlineOptions::singleton.batch_size = 100;

    // RMFE setup
    RmfeShare::setup_rmfe(2, 6);

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

    int n = 10;
    int n_bits = T::bit_type::default_length;
    vector<typename T::bit_type> a(n), b(n);

    // inputs in binary domain
    bit_input.reset_all(P);
    for (int i = 0; i < n; i++)
        bit_input.add_from_all(i + P.my_num(), n_bits);
    bit_input.exchange();
    for (int i = 0; i < n; i++)
    {
        a[i] = bit_input.finalize(0, n_bits);
        b[i] = bit_input.finalize(1, n_bits);
    }

    // compute AND in binary domain
    bit_protocol.init_mul();
    for (int i = 0; i < n; i++)
        bit_protocol.prepare_mul(a[i], b[i], n_bits);
    bit_protocol.exchange();
    bit_protocol.check();
    bit_output.init_open(P, n);
    PointerVector<dabitpack<T>> dabitpacks;
    for (int i = 0; i < n; i++)
    {
        auto c = bit_protocol.finalize_mul(n_bits);
        dabitpacks.push_back({});
        auto& dv = dabitpacks.back();
        dv = prep.get_dabitpack();

        // mask result with dabits and open
        bit_output.prepare_open(c + dv.second);
    }

    bit_output.exchange(P);

    output.init_open(P, n);
    for (int i = 0; i < n; i++)
    {
        T res;
        // unmask via XOR and recombine
        typename T::bit_type::clear masked = bit_output.finalize_open();
        auto& dv = dabitpacks.next();
        for (int j = 0; j < n_bits; j++)
        {
            auto mask = dv.first[j];
            res += (mask - mask * masked.get_bit(j) * 2
                    + T::constant(masked.get_bit(j), P.my_num(), setup.get_mac_key()))
                    << j;
        }
        output.prepare_open(res);
    }
    output.exchange(P);
    set.check();

    cout << "result: ";
    for (int i = 0; i < n; i++)
        cout << output.finalize_open() << " ";
    cout << endl;

    set.check();
}
