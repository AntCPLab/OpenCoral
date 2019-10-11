/*
 * replicated-party.cpp
 *
 */

#include "GC/ShareParty.h"

#include "GC/ShareParty.hpp"
#include "GC/ShareSecret.hpp"
#include "GC/Instruction.hpp"
#include "GC/Machine.hpp"
#include "GC/Processor.hpp"
#include "GC/Program.hpp"
#include "GC/Thread.hpp"
#include "GC/ThreadMaster.hpp"

#include "Processor/Machine.hpp"
#include "Processor/Instruction.hpp"
#include "Protocols/MaliciousRepMC.hpp"
#include "Protocols/MAC_Check_Base.hpp"
#include "Protocols/Beaver.hpp"

int main(int argc, const char** argv)
{
    GC::ShareParty<GC::SemiHonestRepSecret>(argc, argv);
}
