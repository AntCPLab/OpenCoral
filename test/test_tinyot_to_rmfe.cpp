
#include "TinyOT/tinyot.h"
#include "Protocols/TinyOt2Rmfe.h"
#include "Protocols/ProtocolSet.h"
#include "Math/Bit.h"
#include "Machines/Rmfe.hpp"


typedef GC::RmfeSecret T;

void test_tinyot_to_rmfe(int argc, char** argv)
{
    // set up networking on localhost
    int my_number = atoi(argv[1]);
    int n_parties = atoi(argv[2]);
    int port_base = 9999;
    Names N(my_number, n_parties, "localhost", port_base);
    PlainPlayer P(N);

    // RMFE setup
    auto rmfe = get_composite_gf2_rmfe_type2(2, 6);
    rmfe->set_singleton(rmfe.get());

    // protocol setup (domain, MAC key if needed etc)
    BinaryProtocolSetup<T> setup(P);

    // set of protocols (input, multiplication, output)
    BinaryProtocolSet<T> set(P, setup);
    auto& prep = set.prep;
    auto& output = set.output;
    auto& input = set.input;
    auto& protocol = set.protocol;

    // Setup tiny ot
    BufferTinyOTPrep tinyot_prep(my_number, 12345);
    TinyOTMC tinyotMC;
    TinyOt2Rmfe tinyot2rmfe(&tinyotMC, &tinyot_prep);

    int n = 1024;
    int l = GC::RmfeShare::default_length;
    vector<TinyOTShare> tinyot_shares(n * l);
    vector<GC::RmfeShare> rmfe_shares;

    // Generate some tinyot bits
    for(size_t i = 0; i < tinyot_shares.size(); i++) {
        tinyot_prep.get_random_abit(tinyot_shares[i].MAC, tinyot_shares[i].KEY);
    }

    tinyot2rmfe.convert(rmfe_shares, tinyot_shares);
    
    // TinyOT opens
    tinyotMC.init_open(P, n);
    for (int i = 0; i < n * l; i++)
    {
        tinyotMC.prepare_open(tinyot_shares[i]);
    }
    tinyotMC.exchange(P);

    cout << "tinyot opens: " << hex;
    for (int i = 0; i < n; i++) {
        int x = 0;
        for (int j = 0; j < l; j++) {
            x += ((int) tinyotMC.finalize_open().get()) << j;
        }
        cout << x << " ";
    }
    cout << endl;

    // Rmfe opens
    output.init_open(P, n);
    for (int i = 0; i < n; i++)
    {
        output.prepare_open(rmfe_shares[i]);
    }
    output.exchange(P);

    cout << "rmfe opens: " << hex;
    for (int i = 0; i < n; i++)
        cout << output.finalize_open() << " ";
    cout << dec << endl;

    auto comm_stats = P.total_comm();
    size_t rounds = 0;
    for (auto& x : comm_stats)
      rounds += x.second.rounds;
    std::cerr << "Data sent = " << comm_stats.sent / 1e6 << " MB in ~" << rounds
        << " rounds (party " << my_number << std::endl;;
    std::cout << "field degree: " << gf2n_rmfe::degree() << std::endl;
}

int main(int argc, char** argv)
{
    test_tinyot_to_rmfe(argc, argv);
}