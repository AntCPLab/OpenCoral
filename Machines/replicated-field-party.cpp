/*
 * replicated-field-party.cpp
 *
 */

#include "Math/gfp.hpp"
#include "Protocols/ReplicatedFieldMachine.hpp"
#include "Machines/Rep.hpp"

int main(int argc, const char** argv)
{
    ReplicatedFieldMachine<Rep3Share>(argc, argv);
}
