/*
 * malicious-rep-field-party.cpp
 *
 */

#include "Math/MaliciousRep3Share.h"
#include "Processor/ReplicatedMachine.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    ReplicatedMachine<MaliciousRep3Share<gfp>, MaliciousRep3Share<gf2n>>(argc,
            argv, "malicious-rep-field", opt);
}
