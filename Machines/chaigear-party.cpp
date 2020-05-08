/*
 * chaigear-party.cpp
 *
 */

#include "Protocols/ChaiGearShare.h"

#include "Player-Online.hpp"
#include "SPDZ.hpp"
#include "Protocols/ChaiGearPrep.hpp"
#include "Math/gfp.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    CowGearOptions::singleton = CowGearOptions(opt, argc, argv);
    spdz_main<ChaiGearShare<gfp>, ChaiGearShare<gf2n_short>>(argc, argv, opt);
}
