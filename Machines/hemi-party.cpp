/*
 * hemi-party.cpp
 *
 */

#include "Protocols/HemiShare.h"
#include "Math/gfp.h"
#include "Math/gf2n.h"
#include "FHE/P2Data.h"
#include "Tools/ezOptionParser.h"

#include "Player-Online.hpp"
#include "Protocols/HemiPrep.hpp"
#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"
#include "Protocols/SemiPrep.hpp"
#include "Protocols/SemiInput.hpp"
#include "Protocols/MAC_Check_Base.hpp"
#include "Protocols/MAC_Check.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Protocols/SemiMC.hpp"
#include "Protocols/Beaver.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    spdz_main<HemiShare<gfp>, HemiShare<gf2n_short>>(argc, argv, opt);
}
