/*
 * RingOptions.h
 *
 */

#ifndef PROCESSOR_RINGOPTIONS_H_
#define PROCESSOR_RINGOPTIONS_H_

#include "Tools/ezOptionParser.h"

class RingOptions
{
public:
    int R;

    RingOptions(ez::ezOptionParser& opt, int argc, const char** argv);
};

#endif /* PROCESSOR_RINGOPTIONS_H_ */
