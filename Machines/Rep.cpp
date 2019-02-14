/*
 * Rep.cpp
 *
 */

#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"

template<>
Preprocessing<Rep3Share<gfp>>* Preprocessing<Rep3Share<gfp>>::get_live_prep(
    SubProcessor<Rep3Share<gfp>>* proc)
{
  return new ReplicatedPrep<Rep3Share<gfp>>(proc);
}

template<>
Preprocessing<Rep3Share<gf2n>>* Preprocessing<Rep3Share<gf2n>>::get_live_prep(
    SubProcessor<Rep3Share<gf2n>>* proc)
{
  return new ReplicatedPrep<Rep3Share<gf2n>>(proc);
}

template<>
Preprocessing<Rep3Share<Integer>>* Preprocessing<Rep3Share<Integer>>::get_live_prep(
    SubProcessor<Rep3Share<Integer>>* proc)
{
  return new ReplicatedRingPrep<Rep3Share<Integer>>(proc);
}

template<>
Preprocessing<MaliciousRep3Share<gfp>>* Preprocessing<MaliciousRep3Share<gfp>>::get_live_prep(
    SubProcessor<MaliciousRep3Share<gfp>>* proc)
{
  (void) proc;
  return new MaliciousRepPrep<MaliciousRep3Share<gfp>>(proc);
}

template<>
Preprocessing<MaliciousRep3Share<gf2n>>* Preprocessing<MaliciousRep3Share<gf2n>>::get_live_prep(
    SubProcessor<MaliciousRep3Share<gf2n>>* proc)
{
  (void) proc;
  return new MaliciousRepPrep<MaliciousRep3Share<gf2n>>(proc);
}

template class Machine<Rep3Share<Integer>, Rep3Share<gf2n>>;
template class Machine<Rep3Share<gfp>, Rep3Share<gf2n>>;
template class Machine<MaliciousRep3Share<gfp>, MaliciousRep3Share<gf2n>>;
