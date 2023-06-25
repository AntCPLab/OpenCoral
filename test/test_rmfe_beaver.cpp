
#include "Protocols/ProtocolSet.h"
#include "Math/Bit.h"
#include "Machines/Rmfe.hpp"


typedef GC::RmfeSecret<Bit, gf2n_mac_key> T;

void test_rmfe_beaver(int argc, char** argv)
{
    Bit x;
    gf2n_mac_key y;
    cout << "test mul: " << x * y << endl;
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
    auto& output = set.output;
    auto& input = set.input;
    auto& protocol = set.protocol;
    
    int n_bits = 16;
    int n = 10;
    vector<T> a(n), b(n);

    input.reset_all(P);
    for (int i = 0; i < n; i++)
        input.add_from_all(i + P.my_num(), n_bits);
    input.exchange();
    for (int i = 0; i < n; i++)
    {
        a[i] = input.finalize(0, n_bits);
        b[i] = input.finalize(1, n_bits);
    }
    cout << "a[0]: " << a[0] << endl;
    for (int i = 0; i < n_bits; i++)
        cout << a[0].get_bit(i).get_share();
    cout << endl;

    protocol.init_mul();
    for (int i = 0; i < n; i++)
        protocol.prepare_mul(a[i], b[i], n_bits);
    protocol.exchange();
    output.init_open(P, n);
    // for (int i = 0; i < n; i++)
    // {
    //     auto c = protocol.finalize_mul(n_bits);
    //     output.prepare_open(c);
    // }
    for (int i = 0; i < n; i++)
    {
        output.prepare_open(a[i]);
    }
    output.exchange(P);
    set.check();

    cout << "result: ";
    for (int i = 0; i < n; i++)
        cout << output.finalize_open() << " ";
    cout << endl;

    auto comm_stats = P.total_comm();
    size_t rounds = 0;
    for (auto& x : comm_stats)
      rounds += x.second.rounds;
    std::cerr << "Data sent = " << comm_stats.sent / 1e6 << " MB in ~" << rounds
        << " rounds (party " << my_number << std::endl;;
}

int main(int argc, char** argv)
{
    test_rmfe_beaver(argc, argv);
}