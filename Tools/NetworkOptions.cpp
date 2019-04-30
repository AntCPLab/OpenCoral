/*
 * NetworkOptions.cpp
 *
 */

#include "NetworkOptions.h"

NetworkOptions::NetworkOptions(ez::ezOptionParser& opt, int argc,
        const char** argv)
{
    opt.add(
            "localhost", // Default.
            0, // Required?
            1, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Host where party 0 is running (default: localhost)", // Help description.
            "-h", // Flag token.
            "--hostname" // Flag token.
    );
    opt.add(
            "5000", // Default.
            0, // Required?
            1, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Base port number (default: 5000).", // Help description.
            "-pn", // Flag token.
            "--portnum" // Flag token.
    );
    opt.parse(argc, argv);
    opt.get("-pn")->getInt(portnum_base);
    opt.get("-h")->getString(hostname);
    opt.resetArgs();
}
