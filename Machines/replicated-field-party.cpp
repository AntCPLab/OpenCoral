/*
 * replicated-field-party.cpp
 *
 */

#include "Math/gfp.hpp"
#include "Protocols/ReplicatedMachine.hpp"
#include "Machines/Rep.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    ReplicatedMachine<Rep3Share<gfp>, Rep3Share<gf2n>>(argc, argv,
            "replicated-field", opt);
}
