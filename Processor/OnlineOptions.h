/*
 * OnlineOptions.h
 *
 */

#ifndef PROCESSOR_ONLINEOPTIONS_H_
#define PROCESSOR_ONLINEOPTIONS_H_

#include "Tools/ezOptionParser.h"
#include "Math/bigint.h"

class OnlineOptions
{
public:
    static OnlineOptions singleton;

    bool interactive;
    int lgp;
    bigint prime;
    bool live_prep;
    int playerno;
    std::string progname;
    int batch_size;
    std::string memtype;
    bool direct;
    int bucket_size;

    OnlineOptions();
    OnlineOptions(ez::ezOptionParser& opt, int argc, const char** argv,
            int default_batch_size = 0, bool default_live_prep = true,
            bool variable_prime_length = false);
    void finalize(ez::ezOptionParser& opt, int argc, const char** argv);
};

#endif /* PROCESSOR_ONLINEOPTIONS_H_ */
