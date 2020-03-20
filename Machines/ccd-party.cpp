/*
 * ccd-party.cpp
 *
 */

#include "GC/CcdSecret.h"
#include "GC/TinyMC.h"
#include "GC/CcdPrep.h"
#include "GC/VectorInput.h"

#include "GC/ShareParty.hpp"
#include "GC/ShareSecret.hpp"
#include "GC/ThreadMaster.hpp"
#include "GC/Secret.hpp"
#include "Machines/ShamirMachine.hpp"

int main(int argc, const char** argv)
{
    gf2n_short::init_field(40);
    GC::ShareParty<GC::CcdSecret<gf2n_short>>(argc, argv);
}
