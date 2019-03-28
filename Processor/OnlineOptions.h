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
    bool live_prep;
    int playerno;
    std::string progname;

    OnlineOptions();
    OnlineOptions(ez::ezOptionParser& opt, int argc, const char** argv);
    void finalize(ez::ezOptionParser& opt, int argc, const char** argv);
};

#endif /* PROCESSOR_ONLINEOPTIONS_H_ */
