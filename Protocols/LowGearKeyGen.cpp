/*
 * LowGearKeyGen.cpp
 *
 */

#include "FHEOffline/DataSetup.h"
#include "Processor/OnlineOptions.h"

#include "Protocols/LowGearKeyGen.hpp"

template<>
void PairwiseSetup<FFT_Data>::key_and_mac_generation(Player& P,
        PairwiseMachine& machine, int, false_type)
{
    int n_limbs = params.FFTD()[0].get_prD().get_t();
    switch (n_limbs)
    {
#define X(L) case L: LowGearKeyGen<L>(P, machine, params).run(*this); break;
    X(3) X(4) X(5) X(6)
#undef X
    default:
        throw runtime_error(
                "not compiled for choice of parameters, add X("
                        + to_string(n_limbs) + ") at " + __FILE__ + ":"
                        + to_string(__LINE__ - 5));
    }
}

template<>
void PairwiseSetup<P2Data>::key_and_mac_generation(Player& P,
        PairwiseMachine& machine, int, false_type)
{
    LowGearKeyGen<2>(P, machine, params).run(*this);
}
