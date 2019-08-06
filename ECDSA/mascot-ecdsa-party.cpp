/*
 * fake-spdz-ecdsa-party.cpp
 *
 */

#include "Protocols/Share.hpp"
#include "Protocols/MAC_Check.hpp"
#include "ot-ecdsa-party.hpp"

#include <assert.h>

int main(int argc, const char** argv)
{
    run<Share>(argc, argv);
}
