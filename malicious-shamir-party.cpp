/*
 * malicious-shamir-party.cpp
 *
 */

#include "Processor/ShamirMachine.h"
#include "Math/MaliciousShamirShare.h"

int main(int argc, const char** argv)
{
    ShamirMachineSpec<MaliciousShamirShare>(argc, argv);
}
