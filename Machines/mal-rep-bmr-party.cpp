/*
 * mal-rep-shamir-party.cpp
 *
 */

#include "Protocols/MaliciousRep3Share.h"

#include "Machines/Rep.hpp"

#include "BMR/RealProgramParty.hpp"

int main(int argc, const char** argv)
{
    RealProgramParty<MaliciousRep3Share<gf2n_long>>(argc, argv);
}
