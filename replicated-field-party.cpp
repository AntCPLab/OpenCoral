/*
 * replicated-field-party.cpp
 *
 */

#include "Processor/ReplicatedMachine.hpp"
#include "Math/gfp.h"

int main(int argc, const char** argv)
{
    ReplicatedMachine<Rep3Share<gfp>, Rep3Share<gf2n>>(argc, argv, "replicated-field");
}
