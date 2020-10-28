/*
 * sy-rep-field-party.cpp
 *
 */

#include "Protocols/SpdzWiseShare.h"
#include "Protocols/MaliciousRep3Share.h"
#include "Protocols/ReplicatedMachine.h"
#include "Protocols/MAC_Check.h"
#include "Protocols/SpdzWiseMC.h"
#include "Protocols/SpdzWisePrep.h"
#include "Protocols/SpdzWiseInput.h"
#include "Math/gfp.h"
#include "Math/gf2n.h"
#include "Tools/ezOptionParser.h"
#include "Processor/NoLivePrep.h"
#include "GC/MaliciousCcdSecret.h"

#include "Protocols/ReplicatedMachine.hpp"
#include "Protocols/Replicated.hpp"
#include "Protocols/MaliciousRepMC.hpp"
#include "Protocols/Share.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Protocols/SpdzWise.hpp"
#include "Protocols/SpdzWisePrep.hpp"
#include "Protocols/SpdzWiseInput.hpp"
#include "Protocols/SpdzWiseShare.hpp"
#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"
#include "GC/ShareSecret.hpp"
#include "GC/RepPrep.hpp"
#include "GC/ThreadMaster.hpp"
#include "Math/gfp.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opts;
    ReplicatedMachine<SpdzWiseShare<MaliciousRep3Share<gfp>>,
            SpdzWiseShare<MaliciousRep3Share<gf2n>>>(argc, argv, opts);
}
