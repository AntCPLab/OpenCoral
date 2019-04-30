/*
 * RingOptions.cpp
 *
 */

#include "RingOptions.h"

#include <iostream>
using namespace std;

RingOptions::RingOptions(ez::ezOptionParser& opt, int argc, const char** argv)
{
    opt.add(
        "64", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Number of integer bits (default: 64)", // Help description.
        "-R", // Flag token.
        "--ring" // Flag token.
    );
    opt.parse(argc, argv);
    opt.get("-R")->getInt(R);
    opt.resetArgs();
    cerr << "Trying to run " << R << "-bit computation" << endl;
}
