/*
 * prep_dir.cpp
 *
 */

#include "Math/Setup.h"
#include "Math/gf2n.h"

string get_prep_dir(int nparties, int lg2p, int gf2ndegree)
{
    if (gf2ndegree == 0)
        gf2ndegree = gf2n::default_length();
    stringstream ss;
    ss << PREP_DIR << nparties << "-" << lg2p << "-" << gf2ndegree << "/";
    return ss.str();
}
