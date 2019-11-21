/*
 * mal-rep-ecdsa-party.cpp
 *
 */

#include "Protocols/Rep3Share.h"

#include "hm-ecdsa-party.hpp"

template<>
Preprocessing<Rep3Share<gfp2>>* Preprocessing<Rep3Share<gfp2>>::get_live_prep(
        SubProcessor<Rep3Share<gfp2>>* proc, DataPositions& usage)
{
    return new ReplicatedPrep<Rep3Share<gfp2>>(proc, usage);
}

int main(int argc, const char** argv)
{
    run<Rep3Share>(argc, argv);
}
