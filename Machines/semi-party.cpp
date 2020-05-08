/*
 * semi-party.cpp
 *
 */

#include "Math/gfp.h"
#include "Protocols/SemiShare.h"
#include "Tools/SwitchableOutput.h"
#include "GC/SemiPrep.h"

#include "Player-Online.hpp"
#include "Semi.hpp"
#include "GC/ShareSecret.hpp"
#include "Math/gfp.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    spdz_main<SemiShare<gfp>, SemiShare<gf2n>>(argc, argv, opt);
}
