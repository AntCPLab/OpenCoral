
#include "Player-Online.hpp"
#include "Math/gfp.hpp"
#include "GC/TinierSecret.h"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    return spdz_main<Share<gfp>, Share<gf2n>>(argc, argv, opt);
}
