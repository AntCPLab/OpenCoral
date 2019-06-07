/*
 * ps-rep-field-party.cpp
 *
 */

#include "Protocols/PostSacriRepFieldShare.h"
#include "Protocols/ReplicatedMachine.hpp"
#include "Machines/Rep.hpp"
#include "Protocols/PostSacrifice.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    ReplicatedMachine<PostSacriRepFieldShare<gfp>, PostSacriRepFieldShare<gf2n>>(
            argc, argv, opt);
}
