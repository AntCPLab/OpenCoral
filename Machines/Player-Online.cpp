/*
 * Player-Online.cpp
 *
 */

#include "Processor/config.h"
#include "Protocols/Share.h"
#include "GC/TinierSecret.h"
#include "Math/gfp.h"

#include "Player-Online.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    return spdz_main<Share<gfp>, Share<gf2n>>(argc, argv, opt, false);
}
