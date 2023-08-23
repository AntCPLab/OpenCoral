
#include "Protocols/ProtocolSet.h"
#include "Math/Bit.h"
#include "Machines/Rmfe.hpp"
#include "Tools/debug.hpp"


typedef GC::RmfeShare T;

void test_rmfe_buffer_normal(int argc, char** argv)
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

    prep.buffer_normals();
}

int main(int argc, char** argv)
{
    test_rmfe_buffer_normal(argc, argv);
}