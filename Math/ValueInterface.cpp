/*
 * ValueInterface.cpp
 *
 */

#include "ValueInterface.h"
#include "gf2n.h"
#include "Setup.h"

void ValueInterface::read_setup(int nparties, int lg2p, int gf2ndegree)
{
    (void) nparties, (void) lg2p;
    init_gf2n(gf2ndegree);
}
