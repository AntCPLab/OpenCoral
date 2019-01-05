/*
 * shamir-party.cpp
 *
 */

#include "Processor/ShamirMachine.h"
#include "Math/ShamirShare.h"

int main(int argc, const char** argv)
{
    ShamirMachineSpec<ShamirShare>(argc, argv);
}
