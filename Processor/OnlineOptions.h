/*
 * OnlineOptions.h
 *
 */

#ifndef PROCESSOR_ONLINEOPTIONS_H_
#define PROCESSOR_ONLINEOPTIONS_H_

#include "Tools/ezOptionParser.h"

class OnlineOptions
{
public:
    bool interactive;
    int lgp;

    OnlineOptions();
    OnlineOptions(ez::ezOptionParser& opt, int argc, const char** argv);
};

#endif /* PROCESSOR_ONLINEOPTIONS_H_ */
