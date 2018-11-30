/*
 * OnlineOptions.cpp
 *
 */

#include "OnlineOptions.h"

OnlineOptions::OnlineOptions()
{
    interactive = false;
}

OnlineOptions::OnlineOptions(ez::ezOptionParser& opt, int argc,
        const char** argv)
{
    opt.add(
          "", // Default.
          0, // Required?
          0, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Interactive mode in the main thread (default: disabled)", // Help description.
          "-I", // Flag token.
          "--interactive" // Flag token.
    );

    opt.parse(argc, argv);

    interactive = opt.isSet("-I");

    opt.resetArgs();
}
