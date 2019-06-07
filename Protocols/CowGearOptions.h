/*
 * CowGearOptions.h
 *
 */

#ifndef PROTOCOLS_COWGEAROPTIONS_H_
#define PROTOCOLS_COWGEAROPTIONS_H_

#include "Tools/ezOptionParser.h"

class CowGearOptions
{
    void lowgear_from_covert();

public:
    static CowGearOptions singleton;

    int covert_security;
    int lowgear_security;

    CowGearOptions();
    CowGearOptions(ez::ezOptionParser& opt, int argc, const char** argv);
};

#endif /* PROTOCOLS_COWGEAROPTIONS_H_ */
