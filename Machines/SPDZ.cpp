#include "Math/Spdz2kShare.h"

#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"
#include "Auth/MAC_Check.hpp"

#include "Processor/MascotPrep.hpp"
#include "Processor/Spdz2kPrep.hpp"

#ifdef USE_GF2N_LONG
template<>
Preprocessing<Share<gfp>>* Preprocessing<Share<gfp>>::get_live_prep(
        SubProcessor<Share<gfp>>* proc, DataPositions& usage)
{
    return new MascotFieldPrep<Share<gfp>>(proc, usage);
}

template<>
Preprocessing<Share<gf2n_long>>* Preprocessing<Share<gf2n_long>>::get_live_prep(
        SubProcessor<Share<gf2n_long>>* proc, DataPositions& usage)
{
    return new MascotFieldPrep<Share<gf2n>>(proc, usage);
}

template<>
Preprocessing<Spdz2kShare<64, 64>>* Preprocessing<Spdz2kShare<64, 64>>::get_live_prep(
        SubProcessor<Spdz2kShare<64, 64>>* proc, DataPositions& usage)
{
    return new Spdz2kPrep<Spdz2kShare<64, 64>>(proc, usage);
}

template<>
Preprocessing<Spdz2kShare<64, 48>>* Preprocessing<Spdz2kShare<64, 48>>::get_live_prep(
        SubProcessor<Spdz2kShare<64, 48>>* proc, DataPositions& usage)
{
    return new Spdz2kPrep<Spdz2kShare<64, 48>>(proc, usage);
}
#endif

template class Machine<sgfp, Share<gf2n>>;

template class Machine<Spdz2kShare<64, 64>, Share<gf2n>>;
template class Machine<Spdz2kShare<64, 48>, Share<gf2n>>;
