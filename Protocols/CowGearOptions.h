/*
 * CowGearOptions.h
 *
 */

#ifndef PROTOCOLS_COWGEAROPTIONS_H_
#define PROTOCOLS_COWGEAROPTIONS_H_

#include "Tools/ezOptionParser.h"

class CowGearOptions
{
    bool use_top_gear;

    void lowgear_from_covert();

public:
    static CowGearOptions singleton;

    int covert_security;
    int lowgear_security;

    CowGearOptions();
    CowGearOptions(ez::ezOptionParser& opt, int argc, const char** argv);

    bool top_gear()
    {
        return use_top_gear;
    }
};

#endif /* PROTOCOLS_COWGEAROPTIONS_H_ */
