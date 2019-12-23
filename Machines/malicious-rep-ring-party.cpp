/*
 * mal-rep-ring-party.cpp
 *
 */

#include "Protocols/MalRepRingShare.h"
#include "Protocols/MalRepRingOptions.h"
#include "Protocols/ReplicatedMachine.hpp"
#include "Processor/RingOptions.h"
#include "Machines/RepRing.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    MalRepRingOptions::singleton = MalRepRingOptions(opt, argc, argv);
    RingOptions opts(opt, argc, argv);
    switch (opts.R)
    {
    case 64:
        ReplicatedMachine<MalRepRingShare<64, 40>, MaliciousRep3Share<gf2n>>(
                argc, argv, opt);
        break;
    case 72:
        ReplicatedMachine<MalRepRingShare<72, 40>, MaliciousRep3Share<gf2n>>(
                argc, argv, opt);
        break;
    default:
        throw runtime_error(to_string(opts.R) + "-bit computation not implemented");
    }
}
