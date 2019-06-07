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

#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"
#include "Protocols/MAC_Check.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Protocols/Beaver.hpp"
#include "Protocols/Share.hpp"

#include "Player-Online.hpp"

#include "Protocols/CowGearPrep.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    CowGearOptions::singleton = CowGearOptions(opt, argc, argv);
    spdz_main<CowGearShare<gfp>, CowGearShare<gf2n_short>>(argc, argv, opt);
}
