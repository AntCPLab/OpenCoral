/*
 * mal-rep-ring-party.cpp
 *
 */

#include "Protocols/PostSacriRepRingShare.h"
#include "Protocols/PostSacriRepFieldShare.h"
#include "Protocols/ReplicatedMachine.hpp"
#include "Processor/RingOptions.h"
#include "Machines/RepRing.hpp"
#include "Protocols/PostSacrifice.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    RingOptions opts(opt, argc, argv);
    switch (opts.R)
    {
    case 64:
        ReplicatedMachine<PostSacriRepRingShare<64, 40>, PostSacriRepFieldShare<gf2n>>(
                argc, argv, opt);
        break;
    case 72:
        ReplicatedMachine<PostSacriRepRingShare<72, 40>, PostSacriRepFieldShare<gf2n>>(
                argc, argv, opt);
        break;
    default:
        throw runtime_error(to_string(opts.R) + "-bit computation not implemented");
    }
}
