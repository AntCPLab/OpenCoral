/*
 * KeyGen.cpp
 *
 */

#include "FHEOffline/DataSetup.h"
#include "Processor/OnlineOptions.h"

#include "Protocols/HighGearKeyGen.hpp"

template<>
void PartSetup<FFT_Data>::key_and_mac_generation(Player& P,
        MachineBase& machine, int, false_type)
{
    auto& batch_size = OnlineOptions::singleton.batch_size;
    auto backup = batch_size;
    batch_size = 100;
    bool done = false;
    int n_limbs[2];
    for (int i = 0; i < 2; i++)
        n_limbs[i] = params.FFTD()[i].get_prD().get_t();
#define X(L, M) \
    if (n_limbs[0] == L and n_limbs[1] == M) \
    { \
        HighGearKeyGen<L, M>(P, params).run(*this, machine); \
        done = true; \
    }
    X(5, 3) X(4, 3) X(3, 2)
    if (not done)
        throw runtime_error("not compiled for choice of parameters");
    batch_size = backup;
}

template<>
void PartSetup<P2Data>::key_and_mac_generation(Player& P, MachineBase& machine,
        int, false_type)
{
    HighGearKeyGen<2, 2>(P, params).run(*this, machine);
}
