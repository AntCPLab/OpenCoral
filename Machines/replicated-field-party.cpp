/*
 * replicated-field-party.cpp
 *
 */

#include "Protocols/ReplicatedMachine.hpp"
#include "Machines/Rep.hpp"
#include "Math/gfp.h"

template<>
Preprocessing<Rep3Share<gfp>>* Preprocessing<Rep3Share<gfp>>::get_live_prep(
		SubProcessor<Rep3Share<gfp>>* proc, DataPositions& usage)
{
	return new ReplicatedPrep<Rep3Share<gfp>>(proc, usage);
}

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    ReplicatedMachine<Rep3Share<gfp>, Rep3Share<gf2n>>(argc, argv,
            "replicated-field", opt);
}
