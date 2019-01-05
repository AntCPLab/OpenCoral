/*
 * replicated-ring-party.cpp
 *
 */

#include "Processor/ReplicatedMachine.hpp"
#include "Math/Integer.h"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    ReplicatedMachine<Rep3Share<Integer>, Rep3Share<gf2n>>(argc, argv,
            "replicated-ring", opt);
}
