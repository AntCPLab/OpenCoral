/*
 * cowgear-party.cpp
 *
 */

#include "Protocols/CowGearShare.h"
#include "Protocols/CowGearPrep.h"
#include "Protocols/CowGearOptions.h"

#include "FHE/FHE_Params.h"
#include "FHE/FFT_Data.h"
#include "FHE/NTL-Subs.h"

#include "GC/TinierSecret.h"
#include "GC/TinierPrep.h"
#include "GC/TinyMC.h"

#include "SPDZ.hpp"
#include "Player-Online.hpp"

#include "Protocols/CowGearPrep.hpp"
#include "Math/gfp.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    CowGearOptions::singleton = CowGearOptions(opt, argc, argv);
    spdz_main<CowGearShare<gfp>, CowGearShare<gf2n_short>>(argc, argv, opt);
}
