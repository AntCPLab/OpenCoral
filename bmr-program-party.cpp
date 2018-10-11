// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * bmr-program-party.cpp
 *
 */

#include "BMR/Party.h"

int main(int argc, char** argv)
{
	ProgramParty party(argc, argv);
	party.Start();
}
