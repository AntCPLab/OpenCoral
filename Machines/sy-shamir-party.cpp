/*
 * sy-shamir-party.cpp
 *
 */

#include "ShamirMachine.h"
#include "Protocols/ReplicatedMachine.h"
#include "Protocols/SpdzWiseShare.h"
#include "Protocols/MaliciousShamirShare.h"
#include "Protocols/SpdzWiseMC.h"
#include "Protocols/SpdzWiseInput.h"
#include "Math/gfp.h"
#include "Math/gf2n.h"
#include "GC/CcdSecret.h"
#include "GC/MaliciousCcdSecret.h"

#include "Protocols/Share.hpp"
#include "Protocols/SpdzWise.hpp"
#include "Protocols/SpdzWisePrep.hpp"
#include "Protocols/SpdzWiseInput.hpp"
#include "Protocols/SpdzWiseShare.hpp"
#include "Machines/ShamirMachine.hpp"

int main(int argc, const char** argv)
{
    auto& opts = ShamirOptions::singleton;
    ez::ezOptionParser opt;
    opts = {opt, argc, argv};
    ReplicatedMachine<SpdzWiseShare<MaliciousShamirShare<gfp>>,
            SpdzWiseShare<MaliciousShamirShare<gf2n>>>(
            argc, argv,
            { }, opt, opts.nparties);
}
