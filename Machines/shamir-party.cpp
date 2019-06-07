/*
 * shamir-party.cpp
 *
 */

#include "Machines/ShamirMachine.h"
#include "Protocols/ShamirShare.h"

int main(int argc, const char** argv)
{
    ShamirMachineSpec<ShamirShare>(argc, argv);
}
