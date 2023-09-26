
#include "TinyOT/tinyot.h"
// #include "Protocols/TinyOt2Rmfe.h"
#include "Protocols/ProtocolSet.h"
#include "Math/Bit.h"
#include "Machines/Rmfe.hpp"
#include "Tools/debug.hpp"
#include "Protocols/RmfeShareConverter.h"
#include "Protocols/ProtocolGlobalInit.h"
#include "GC/RmfeSharePrep.h"

#include "GC/Spdz2kBShare.h"
#include "GC/TinySecret.h"
#include "GC/CcdPrep.hpp"
#include "GC/VectorInput.h"
#include "GC/TinierSharePrep.hpp"
#include "GC/TinierSecret.h"
#include "GC/TinyMC.h"
#include "Protocols/Beaver.hpp"
#include "GC/TinyPrep.hpp"


typedef GC::RmfeShare T;

template<class SrcType>
RmfeShareConverter<SrcType> setup_converter(Player& P) {
    return RmfeShareConverter<SrcType>(P);
}

template<class SrcType>
void test_convert_to_rmfe(int argc, char** argv)
{
    // set up networking on localhost
    int my_number = atoi(argv[1]);
    int n_parties = atoi(argv[2]);
    int port_base = 9999;
    Names N(my_number, n_parties, "localhost", port_base);
    PlainPlayer P(N);

    // RMFE setup
    RmfeShare::setup_rmfe(2, 6);

    // protocol setup (domain, MAC key if needed etc)
    BinaryProtocolSetup<T> setup(P);

    // set of protocols (input, multiplication, output)
    BinaryProtocolSet<T> set(P, setup);
    auto& prep = set.prep;
    auto& output = set.output;
    auto& input = set.input;
    auto& protocol = set.protocol;

    RmfeShareConverter<SrcType> converter = setup_converter<SrcType>(P);
    typename SrcType::MAC_Check* src_mc = converter.get_src_mc();
    typename SrcType::LivePrep* src_prep = converter.get_src_prep();

    int n = 1024;
    int l = GC::RmfeShare::default_length;
    vector<SrcType> src_shares(n * l);
    vector<GC::RmfeShare> rmfe_shares;

    // Generate some src bits
    for(size_t i = 0; i < src_shares.size(); i++) {
        src_shares[i] = src_prep->get_bit();
        // src_prep->get_random_abit(src_shares[i].MAC, src_shares[i].KEY);
    }

    converter.convert(rmfe_shares, src_shares);

    {
        auto comm_stats = P.total_comm();
        size_t rounds = 0;
        for (auto& x : comm_stats)
        rounds += x.second.rounds;
        std::cerr << "1 Data sent = " << comm_stats.sent / 1e6 << " MB in ~" << rounds
            << " rounds (party " << my_number << std::endl;
    }

    
    // Src opens
    src_mc->init_open(P, n);
    for (int i = 0; i < n * l; i++)
    {
        src_mc->prepare_open(src_shares[i]);
    }
    src_mc->exchange(P);

    cout << "src opens: " << hex;
    for (int i = 0; i < n; i++) {
        int x = 0;
        for (int j = 0; j < l; j++) {
            x += ((int) src_mc->finalize_open().get_bit(0)) << j;
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
        cout << output.finalize_open_decoded() << " ";
    cout << dec << endl;

    {
        auto comm_stats = P.total_comm();
        size_t rounds = 0;
        for (auto& x : comm_stats)
        rounds += x.second.rounds;
        std::cerr << "4 Data sent = " << comm_stats.sent / 1e6 << " MB in ~" << rounds
            << " rounds (party " << my_number << std::endl;
    }
}

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

    string protocol = "tinyot";
    if (argc > 3)
        protocol = argv[3];

    if (protocol == "tinyot") {
        test_convert_to_rmfe<TinyOTShare>(argc, argv);
    }
    else if (protocol == "spdz2k") {
        test_convert_to_rmfe<GC::Spdz2kBShare<DEFAULT_SECURITY>>(argc, argv);
    }
}