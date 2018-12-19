/*
 * OnlineOptions.cpp
 *
 */

#include "OnlineOptions.h"

OnlineOptions::OnlineOptions()
{
    interactive = false;
    lgp = 128;
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
    opt.add(
          "128", // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Bit length of GF(p) field (default: 128)", // Help description.
          "-lgp", // Flag token.
          "--lgp" // Flag token.
    );

    opt.parse(argc, argv);

    interactive = opt.isSet("-I");
    opt.get("--lgp")->getInt(lgp);

    opt.resetArgs();
}
