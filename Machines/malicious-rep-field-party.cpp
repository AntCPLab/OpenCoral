/*
 * malicious-rep-field-party.cpp
 *
 */

#include "Protocols/MaliciousRep3Share.h"
#include "Protocols/ReplicatedMachine.hpp"
#include "Machines/Rep.hpp"
#include "Math/gfp.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    ReplicatedMachine<MaliciousRep3Share<gfp>, MaliciousRep3Share<gf2n>>(argc,
            argv, "malicious-rep-field", opt);
}
