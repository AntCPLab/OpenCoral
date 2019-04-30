/*
 * Rep.cpp
 *
 */

#include "Math/MaliciousRep3Share.h"
#include "Math/BrainShare.h"
#include "Processor/BrainPrep.h"

#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"
#include "Processor/BrainPrep.hpp"
#include "Auth/MAC_Check.hpp"
#include "Auth/fake-stuff.hpp"
#include "Auth/MaliciousRepMC.hpp"

template<>
Preprocessing<Rep3Share<gfp>>* Preprocessing<Rep3Share<gfp>>::get_live_prep(
    SubProcessor<Rep3Share<gfp>>* proc, DataPositions& usage)
{
  return new ReplicatedPrep<Rep3Share<gfp>>(proc, usage);
}

template<>
Preprocessing<Rep3Share<gf2n>>* Preprocessing<Rep3Share<gf2n>>::get_live_prep(
    SubProcessor<Rep3Share<gf2n>>* proc, DataPositions& usage)
{
  return new ReplicatedPrep<Rep3Share<gf2n>>(proc, usage);
}

template<>
Preprocessing<Rep3Share<SignedZ2<64>>>* Preprocessing<Rep3Share<SignedZ2<64>>>::get_live_prep(
    SubProcessor<Rep3Share<SignedZ2<64>>>* proc, DataPositions& usage)
{
  return new ReplicatedRingPrep<Rep3Share<SignedZ2<64>>>(proc, usage);
}

template<>
Preprocessing<Rep3Share<SignedZ2<72>>>* Preprocessing<Rep3Share<SignedZ2<72>>>::get_live_prep(
    SubProcessor<Rep3Share<SignedZ2<72>>>* proc, DataPositions& usage)
{
  return new ReplicatedRingPrep<Rep3Share<SignedZ2<72>>>(proc, usage);
}

template class Machine<Rep3Share<SignedZ2<64>>, Rep3Share<gf2n>>;
template class Machine<Rep3Share<SignedZ2<72>>, Rep3Share<gf2n>>;
template class Machine<Rep3Share<gfp>, Rep3Share<gf2n>>;
template class Machine<MaliciousRep3Share<gfp>, MaliciousRep3Share<gf2n>>;
template class Machine<BrainShare<64, 40>, MaliciousRep3Share<gf2n>>;
template class Machine<BrainShare<72, 40>, MaliciousRep3Share<gf2n>>;
