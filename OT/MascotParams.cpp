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
    correlation_check = true;
    generateBits = false;
    use_extension = true;
    fewer_rounds = false;
    fiat_shamir = false;
    timerclear(&start);
}

void MascotParams::set_passive()
{
    generateMACs = amplify = check = correlation_check = false;
}
