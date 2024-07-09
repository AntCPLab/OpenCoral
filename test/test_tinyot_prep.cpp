
#include "TinyOT/tinyot.h"
#include "Protocols/ProtocolSet.h"
#include "Math/Bit.h"
#include "Machines/Rmfe.hpp"
#include "Tools/debug.hpp"
#include "Tools/performance.h"


typedef TinyOTShare T;

void test_tinyot_prep(int argc, char** argv)
{
    // set up networking on localhost
    int my_number = atoi(argv[1]);
    int n_parties = atoi(argv[2]);
    int port_base = 9999;
    Names N(my_number, n_parties, "localhost", port_base);
    PlainPlayer P(N);

    // protocol setup (domain, MAC key if needed etc)
    BinaryProtocolSetup<TinyOTShare> setup(P);

    // set of protocols (input, multiplication, output)
    BinaryProtocolSet<T> set(P, setup);
    auto& prep = set.prep;
    auto& output = set.output;
    auto& input = set.input;
    auto& protocol = set.protocol;

    prep.set_batch_size(20000 * 12);

    auto start = perf_log("tinyot triples", P.total_comm().sent);

    T a, b, c;
    prep.get_tinyot_triple(a.MAC, a.KEY, b.MAC, b.KEY, c.MAC, c.KEY);

    auto diff = perf_log("tinyot triples", P.total_comm().sent);
    cout << "[Time/1000 ops] " << diff.first.count() * 1.0 / 1e6 / prep.get_batch_size() * 1000 << " ms" << endl;
    cout << "[Comm/1000 ops] " << diff.second * 1.0 / 1e6 / prep.get_batch_size() * 1000 << " MB" << endl;
}

int main(int argc, char** argv)
{
    test_tinyot_prep(argc, argv);
}