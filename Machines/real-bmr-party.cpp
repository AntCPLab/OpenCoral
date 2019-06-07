/*
 * real-bmr-party.cpp
 *
 */

#include "Machines/SPDZ.cpp"

#include "BMR/RealProgramParty.hpp"

int main(int argc, const char** argv)
{
	RealProgramParty<Share<gf2n_long>>(argc, argv);
}
