/*
 * spdz2k-party.cpp
 *
 */

#include "Processor/Machine.h"
#include "Protocols/Spdz2kShare.h"
#include "Math/gf2n.h"
#include "Networking/Server.h"

#include "Player-Online.hpp"
#include "SPDZ2k.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    opt.add(
        "64", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "SPDZ2k security parameter (default: 64)", // Help description.
        "-S", // Flag token.
        "--security" // Flag token.
    );
    opt.parse(argc, argv);
    int s;
    opt.get("-S")->getInt(s);
    opt.resetArgs();
#ifdef VERBOSE
    cerr << "Using SPDZ2k with security parameter " << s << endl;
#endif
    if (s == 64)
        return spdz_main<Spdz2kShare<64, 64>, Share<gf2n>>(argc, argv, opt);
    else if (s == 48)
        return spdz_main<Spdz2kShare<64, 48>, Share<gf2n>>(argc, argv, opt);
    else
        throw runtime_error("not compiled for s=" + to_string(s));
}
