/*
 * replicated-party.cpp
 *
 */

#include "GC/ReplicatedParty.h"

int main(int argc, const char** argv)
{
    GC::ReplicatedParty<GC::SemiHonestRepSecret>(argc, argv);
}
