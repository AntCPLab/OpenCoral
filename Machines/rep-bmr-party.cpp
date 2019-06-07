/*
 * rep-bmr-party.cpp
 *
 */

#include "Machines/Rep.hpp"

#include "BMR/RealProgramParty.hpp"

int main(int argc, const char** argv)
{
    RealProgramParty<Rep3Share<gf2n_long>>(argc, argv);
}
