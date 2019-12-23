/*
 * replicated-ring-party.cpp
 *
 */

#include "Protocols/ReplicatedMachine.hpp"
#include "Protocols/Rep3Share2k.h"
#include "Protocols/ReplicatedPrep2k.h"
#include "Processor/RingOptions.h"
#include "Math/Integer.h"
#include "Machines/RepRing.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    RingOptions opts(opt, argc, argv);
    switch (opts.R)
    {
    case 64:
        ReplicatedMachine<Rep3Share2<64>, Rep3Share<gf2n>>(argc, argv,
                "replicated-ring", opt);
        break;
    case 72:
        ReplicatedMachine<Rep3Share2<72>, Rep3Share<gf2n>>(argc, argv,
                "replicated-ring", opt);
        break;
    default:
        throw runtime_error(to_string(opts.R) + "-bit computation not implemented");
    }
}
