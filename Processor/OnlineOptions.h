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
    static OnlineOptions singleton;

    bool interactive;
    int lgp;
    bool live_prep;
    int playerno;
    std::string progname;
    int batch_size;
    std::string memtype;

    OnlineOptions();
    OnlineOptions(ez::ezOptionParser& opt, int argc, const char** argv,
            int default_batch_size = 0, bool default_live_prep = true);
    void finalize(ez::ezOptionParser& opt, int argc, const char** argv);
};

#endif /* PROCESSOR_ONLINEOPTIONS_H_ */
