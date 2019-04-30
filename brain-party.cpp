/*
 * brain-party.cpp
 *
 */

#include "Math/BrainShare.h"
#include "Math/MaliciousRep3Share.h"
#include "Processor/RingOptions.h"

#include "Processor/ReplicatedMachine.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    RingOptions opts(opt, argc, argv);
    switch (opts.R)
    {
    case 64:
        // multiple of eight for quicker randomness generation
        gfp2::init_default(DIV_CEIL(BrainShare<64, 40>::Z_BITS + 3, 8) * 8);
        ReplicatedMachine<BrainShare<64, 40>, MaliciousRep3Share<gf2n>>(argc,
                argv, "", opt);
        break;
    case 72:
        // multiple of eight for quicker randomness generation
        gfp2::init_default(DIV_CEIL(BrainShare<72, 40>::Z_BITS + 3, 8) * 8);
        ReplicatedMachine<BrainShare<72, 40>, MaliciousRep3Share<gf2n>>(argc,
                argv, "", opt);
        break;
    default:
        throw runtime_error(to_string(opts.R) + "-bit computation not implemented");
    }
}
