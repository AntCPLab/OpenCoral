/*
 * semi-party.cpp
 *
 */

#include "Math/gfp.h"
#include "Protocols/SemiShare.h"

#include "Player-Online.hpp"
#include "Semi.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    spdz_main<SemiShare<gfp>, SemiShare<gf2n>>(argc, argv, opt);
}
