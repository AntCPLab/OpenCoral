/*
 * shamir-bmr-party.cpp
 *
 */

#include "Machines/ShamirMachine.hpp"

#include "BMR/RealProgramParty.hpp"
#include "Math/Z2k.hpp"

int main(int argc, const char** argv)
{
	ez::ezOptionParser opt;
	ShamirOptions::singleton = {opt, argc, argv};
	RealProgramParty<ShamirShare<gf2n_long>>(argc, argv);
}
