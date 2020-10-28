/*
 * rep4-party.cpp
 *
 */

#include "Protocols/Rep4Share2k.h"
#include "Protocols/Rep4Share.h"
#include "Protocols/Rep4MC.h"
#include "Protocols/ReplicatedMachine.h"
#include "Math/Z2k.h"
#include "Math/gf2n.h"
#include "Tools/ezOptionParser.h"
#include "GC/Rep4Secret.h"
#include "Processor/RingOptions.h"

#include "Protocols/RepRingOnlyEdabitPrep.hpp"
#include "Protocols/ReplicatedMachine.hpp"
#include "Protocols/Rep4Input.hpp"
#include "Protocols/Rep4Prep.hpp"
#include "Protocols/Rep4MC.hpp"
#include "Protocols/Rep4.hpp"
#include "GC/BitAdder.hpp"
#include "Math/Z2k.hpp"
#include "Rep.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    RingOptions ring_opts(opt, argc, argv);
    switch (ring_opts.R)
    {
#define X(R) case R: ReplicatedMachine<Rep4Share2<R>, Rep4Share<gf2n>>(argc, argv, opt, 4); break;
    X(64) X(80) X(88)
    default:
        cerr << ring_opts.R << "-bit computation not implemented" << endl;
        exit(1);
    }
}
