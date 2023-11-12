/*
 * corallowgear-party.cpp
 *
 */

#include "Protocols/CoralLowGearShare.h"

#include "SPDZ.hpp"
#include "Protocols/CowGearPrep.hpp"
#include "Coral.hpp"

int main(int argc, const char** argv)
{
    const char** argv_hooked = new const char*[argc+2];
    for (int i = 0; i < argc; i++) {
        argv_hooked[i] = argv[i];
    }
    argv_hooked[argc] = "--lg2";
    string lg2 = to_string(RmfeShare::open_type::default_degree());
    argv_hooked[argc + 1] = lg2.data();
    int argc_hooked = argc + 2;

    ez::ezOptionParser opt;
    CowGearOptions::singleton = CowGearOptions(opt, argc_hooked, argv_hooked, false);
    DishonestMajorityFieldMachine<CoralLowGearShare, CoralLowGearShare, gf2n_short>(argc_hooked, argv_hooked, opt);

    delete[] argv_hooked;
}
