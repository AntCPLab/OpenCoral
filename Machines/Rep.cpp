/*
 * Rep.cpp
 *
 */

#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"
#include "Auth/MAC_Check.hpp"

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
Preprocessing<Rep3Share<Integer>>* Preprocessing<Rep3Share<Integer>>::get_live_prep(
    SubProcessor<Rep3Share<Integer>>* proc, DataPositions& usage)
{
  return new ReplicatedRingPrep<Rep3Share<Integer>>(proc, usage);
}

template<>
Preprocessing<MaliciousRep3Share<gfp>>* Preprocessing<MaliciousRep3Share<gfp>>::get_live_prep(
    SubProcessor<MaliciousRep3Share<gfp>>* proc, DataPositions& usage)
{
  (void) proc;
  return new MaliciousRepPrep<MaliciousRep3Share<gfp>>(proc, usage);
}

template<>
Preprocessing<MaliciousRep3Share<gf2n>>* Preprocessing<MaliciousRep3Share<gf2n>>::get_live_prep(
    SubProcessor<MaliciousRep3Share<gf2n>>* proc, DataPositions& usage)
{
  (void) proc;
  return new MaliciousRepPrep<MaliciousRep3Share<gf2n>>(proc, usage);
}

template class Machine<Rep3Share<Integer>, Rep3Share<gf2n>>;
template class Machine<Rep3Share<gfp>, Rep3Share<gf2n>>;
template class Machine<MaliciousRep3Share<gfp>, MaliciousRep3Share<gf2n>>;
