#define VERBOSE_DEBUG_PRINT

#include "Machines/Rmfe.hpp"

#include "Tools/debug.hpp"
#include "GC/TinierSecret.h"
#include "GC/PostSacriSecret.h"
#include "GC/CcdSecret.h"
#include "GC/MaliciousCcdSecret.h"
#include "GC/AtlasSecret.h"
#include "GC/TinyMC.h"
#include "GC/VectorInput.h"
#include "GC/PostSacriBin.h"
#include "Protocols/ProtocolSet.h"

#include "GC/ShareSecret.hpp"
#include "GC/CcdPrep.hpp"
#include "GC/TinierSharePrep.hpp"
#include "GC/RepPrep.hpp"
#include "GC/Secret.hpp"
#include "GC/TinyPrep.hpp"
#include "GC/ThreadMaster.hpp"
#include "GC/SemiSecret.hpp"
#include "Protocols/Atlas.hpp"
#include "Protocols/MaliciousRepPrep.hpp"
#include "Protocols/Share.hpp"
#include "Protocols/MaliciousRepMC.hpp"
#include "Protocols/Shamir.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Machines/ShamirMachine.hpp"
#include "Machines/Rep4.hpp"
#include "Machines/Rep.hpp"


template<class T>
void test_buffer_triples(int argc, char** argv)
{
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

    prep.buffer_triples();
    set.check();

    auto comm_stats = P.total_comm();
    size_t rounds = 0;
    for (auto& x : comm_stats)
      rounds += x.second.rounds;
    std::cerr << "Data sent = " << comm_stats.sent / 1e6 << " MB in ~" << rounds
        << " rounds (party " << my_number << std::endl;;
}

template<class T>
void test_buffer_inputs(int argc, char** argv)
{
    // OnlineOptions::singleton.batch_size = 120000;
    // set up networking on localhost
    int my_number = atoi(argv[1]);
    int n_parties = atoi(argv[2]);
    int port_base = 9999;
    Names N(my_number, n_parties, "localhost", port_base);
    PlainPlayer P(N);

    // protocol setup (domain, MAC key if needed etc)
    BinaryProtocolSetup<T> setup(P);
    cout << "after protocol setup" << endl;
    cout << &Gf2MFE::s() << endl;

    // set of protocols (input, multiplication, output)
    BinaryProtocolSet<T> set(P, setup);
    auto& prep = set.prep;

    prep.buffer_inputs(0);
    set.check();

    auto comm_stats = P.total_comm();
    size_t rounds = 0;
    for (auto& x : comm_stats)
      rounds += x.second.rounds;
    std::cerr << "Data sent = " << comm_stats.sent / 1e6 << " MB in ~" << rounds
        << " rounds (party " << my_number << std::endl;;
}

int main(int argc, char** argv)
{
    // Tinier
    // test_buffer_inputs<GC::TinierSecret<gf2n_mac_key>>(argc, argv);
    // Tiny.
    // test_buffer_inputs<GC::TinySecret<DEFAULT_SECURITY>>(argc, argv);
    // Rmfe
    test_buffer_inputs<GC::RmfeShare>(argc, argv);

    // Tinier
    // test_buffer_triples<GC::TinierSecret<gf2n_mac_key>>(argc, argv);
    // Tiny.
    // test_buffer_triples<GC::TinySecret<DEFAULT_SECURITY>>(argc, argv);
    // Rmfe
    // test_buffer_triples<GC::RmfeShare>(argc, argv);

    // else if (protocol == "Rep3")
    //     run<GC::SemiHonestRepSecret>(argc, argv);
    // else if (protocol == "Rep4")
    //     run<GC::Rep4Secret>(argc, argv);
    // else if (protocol == "PS")
    //     run<GC::PostSacriSecret>(argc, argv);
    // else if (protocol == "Semi")
    //     run<GC::SemiSecret>(argc, argv);
    // else if (protocol == "CCD" or protocol == "MalCCD" or protocol == "Atlas")
    // {
    //     int nparties = (atoi(argv[2]));
    //     int threshold = (nparties - 1) / 2;
    //     if (argc > 5)
    //         threshold = atoi(argv[5]);
    //     assert(2 * threshold < nparties);
    //     ShamirOptions::s().threshold = threshold;
    //     ShamirOptions::s().nparties = nparties;

    //     if (protocol == "CCD")
    //         run<GC::CcdSecret<gf2n_<octet>>>(argc, argv);
    //     else if (protocol == "MalCCD")
    //         run<GC::MaliciousCcdSecret<gf2n_short>>(argc, argv);
    //     else
    //         run<GC::AtlasSecret>(argc, argv);
    // }
    // else
    // {
    //     cerr << "Unknown protocol: " << protocol << endl;
    //     exit(1);
    // }
}