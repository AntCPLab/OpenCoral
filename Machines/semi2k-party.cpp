/*
 * semi2k-party.cpp
 *
 */

#include "Protocols/Semi2kShare.h"
#include "Math/gf2n.h"
#include "Processor/RingOptions.h"

#include "Player-Online.hpp"
#include "Semi.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    RingOptions opts(opt, argc, argv);
    switch (opts.R)
    {
    case 64:
        spdz_main<Semi2kShare<64>, SemiShare<gf2n>>(argc, argv, opt);
        break;
    case 72:
        spdz_main<Semi2kShare<72>, SemiShare<gf2n>>(argc, argv, opt);
        break;
    default:
        throw runtime_error(to_string(opts.R) + "-bit computation not implemented");
    }
}
