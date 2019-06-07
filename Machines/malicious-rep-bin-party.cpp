/*
 * malicious-rep-bin-party.cpp
 *
 */

#include "GC/ReplicatedParty.h"
#include "GC/MaliciousRepSecret.h"

int main(int argc, const char** argv)
{
    GC::ReplicatedParty<GC::MaliciousRepSecret>(argc, argv);
}
