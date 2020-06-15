/*
 * mama-party.cpp
 *
 */

#include "Protocols/MamaShare.h"

#include "Protocols/MamaPrep.hpp"
#include "Protocols/MascotPrep.hpp"
#include "SPDZ.hpp"
#include "Player-Online.hpp"
#include "Math/gfp.hpp"

#ifndef N_MAMA_MACS
#define N_MAMA_MACS 3
#endif

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    return spdz_main<MamaShare<gfp, N_MAMA_MACS>, Share<gf2n>>(argc, argv, opt);
}
