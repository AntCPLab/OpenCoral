/*
 * TripleMachine.cpp
 *
 */

#include <OT/TripleMachine.h>
#include "OT/NPartyTripleGenerator.h"
#include "OT/OTTripleSetup.h"
#include "Math/gf2n.h"
#include "Math/Setup.h"
#include "Protocols/Spdz2kShare.h"
#include "Tools/ezOptionParser.h"
#include "Math/Setup.h"
#include "Protocols/fake-stuff.h"
#include "Math/BitVec.h"

#include "Protocols/fake-stuff.hpp"
#include "Math/Z2k.hpp"

#include <iostream>
#include <fstream>
using namespace std;

MascotParams::MascotParams()
{
    generateMACs = true;
    amplify = true;
    check = true;
    generateBits = false;
    timerclear(&start);
}

void MascotParams::set_passive()
{
    generateMACs = amplify = check = false;
}

template<> gf2n_long MascotParams::get_mac_key()
{
    return mac_key2l;
}

template<> gf2n_short MascotParams::get_mac_key()
{
    return mac_key2s;
}

template<> gfp1 MascotParams::get_mac_key()
{
    return mac_keyp;
}

template<> Z2<48> MascotParams::get_mac_key()
{
    return mac_keyz;
}

template<> Z2<64> MascotParams::get_mac_key()
{
    return mac_keyz;
}

template<> Z2<40> MascotParams::get_mac_key()
{
    return mac_keyz;
}

template<> Z2<32> MascotParams::get_mac_key()
{
    return mac_keyz;
}

template<> BitVec MascotParams::get_mac_key()
{
    return 0;
}

template<> void MascotParams::set_mac_key(gf2n_long key)
{
    mac_key2l = key;
}

template<> void MascotParams::set_mac_key(gf2n_short key)
{
    mac_key2s = key;
}

template<> void MascotParams::set_mac_key(gfp1 key)
{
    mac_keyp = key;
}

template<> void MascotParams::set_mac_key(Z2<64> key)
{
    mac_keyz = key;
}

template<> void MascotParams::set_mac_key(Z2<48> key)
{
    mac_keyz = key;
}

template<> void MascotParams::set_mac_key(Z2<40> key)
{
    mac_keyz = key;
}
