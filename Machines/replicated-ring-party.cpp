/*
 * replicated-ring-party.cpp
 *
 */

#include "Protocols/ReplicatedMachine.hpp"
#include "Processor/RingOptions.h"
#include "Math/Integer.h"
#include "Machines/Rep.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    RingOptions opts(opt, argc, argv);
    switch (opts.R)
    {
    case 64:
        ReplicatedMachine<Rep3Share<SignedZ2<64>>, Rep3Share<gf2n>>(argc, argv,
                "replicated-ring", opt);
        break;
    case 72:
        ReplicatedMachine<Rep3Share<SignedZ2<72>>, Rep3Share<gf2n>>(argc, argv,
                "replicated-ring", opt);
        break;
    default:
        throw runtime_error(to_string(opts.R) + "-bit computation not implemented");
    }
}
