/*
 * NetworkOptions.h
 *
 */

#ifndef TOOLS_NETWORKOPTIONS_H_
#define TOOLS_NETWORKOPTIONS_H_

#include "ezOptionParser.h"

#include <string>

class NetworkOptions
{
public:
    int portnum_base;
    std::string hostname;

    NetworkOptions(ez::ezOptionParser& opt, int argc, const char** argv);
};

#endif /* TOOLS_NETWORKOPTIONS_H_ */
