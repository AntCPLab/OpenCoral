/*
 * sy-rep-ring-party.cpp
 *
 */

#include "Protocols/ReplicatedMachine.h"
#include "Protocols/SpdzWiseRingShare.h"
#include "Protocols/MaliciousRep3Share.h"
#include "Protocols/SpdzWiseMC.h"
#include "Protocols/SpdzWiseRingPrep.h"
#include "Protocols/SpdzWiseInput.h"
#include "Protocols/MalRepRingPrep.h"
#include "Processor/RingOptions.h"
#include "GC/MaliciousCcdSecret.h"

#include "Protocols/ReplicatedMachine.hpp"
#include "Protocols/Replicated.hpp"
#include "Protocols/MaliciousRepMC.hpp"
#include "Protocols/Share.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Protocols/SpdzWise.hpp"
#include "Protocols/SpdzWiseRing.hpp"
#include "Protocols/SpdzWisePrep.hpp"
#include "Protocols/SpdzWiseInput.hpp"
#include "Protocols/SpdzWiseShare.hpp"
#include "Protocols/PostSacrifice.hpp"
#include "Protocols/MalRepRingPrep.hpp"
#include "Protocols/MaliciousRepPrep.hpp"
#include "Protocols/RepRingOnlyEdabitPrep.hpp"
#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"
#include "GC/ShareSecret.hpp"
#include "GC/RepPrep.hpp"
#include "GC/ThreadMaster.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    RingOptions opts(opt, argc, argv, true);
    switch (opts.R)
    {
    case 64:
        switch (opts.S)
        {
        case 40:
            ReplicatedMachine<SpdzWiseRingShare<64, 40>,
                    SpdzWiseShare<MaliciousRep3Share<gf2n>>>(argc, argv, opt);
            break;
        case 64:
            ReplicatedMachine<SpdzWiseRingShare<64, 64>,
                    SpdzWiseShare<MaliciousRep3Share<gf2n>>>(argc, argv, opt);
            break;
        default:
            cerr << "Security parameter " << opts.S << " not implemented"
                    << endl;
            exit(1);
        }
        break;
    case 72:
        ReplicatedMachine<SpdzWiseRingShare<72, 40>,
                SpdzWiseShare<MaliciousRep3Share<gf2n>>>(argc, argv, opt);
        break;
    default:
        throw runtime_error(
                to_string(opts.R) + "-bit computation not implemented");
    }
}
