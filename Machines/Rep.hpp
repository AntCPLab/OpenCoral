/*
 * Rep.cpp
 *
 */

#include "Protocols/MaliciousRep3Share.h"
#include "Protocols/MalRepRingShare.h"
#include "Protocols/BrainShare.h"
#include "Protocols/BrainPrep.h"
#include "Protocols/MalRepRingPrep.h"

#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"
#include "Protocols/BrainPrep.hpp"
#include "Protocols/MalRepRingPrep.hpp"
#include "Protocols/MaliciousRepPrep.hpp"
#include "Protocols/Spdz2kPrep.hpp"
#include "Protocols/MAC_Check_Base.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Protocols/MaliciousRepMC.hpp"
#include "Protocols/Beaver.hpp"
#include "Math/Z2k.hpp"

template<>
Preprocessing<Rep3Share<gf2n>>* Preprocessing<Rep3Share<gf2n>>::get_live_prep(
    SubProcessor<Rep3Share<gf2n>>* proc, DataPositions& usage)
{
  return new ReplicatedPrep<Rep3Share<gf2n>>(proc, usage);
}
