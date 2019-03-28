/*
 * OnlineOptions.cpp
 *
 */

#include "OnlineOptions.h"

using namespace std;

OnlineOptions::OnlineOptions() : playerno(-1)
{
    interactive = false;
    lgp = 128;
    live_prep = true;
}

OnlineOptions::OnlineOptions(ez::ezOptionParser& opt, int argc,
        const char** argv) : OnlineOptions()
{
    opt.syntax = std::string(argv[0]) + " [OPTIONS] [<playerno>] <progname>";

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
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Preprocessing from files", // Help description.
            "-F", // Flag token.
            "--file-preprocessing" // Flag token.
    );
    opt.add(
            "", // Default.
            0, // Required?
            1, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "This player's number (required if not given before program name)", // Help description.
            "-p", // Flag token.
            "--player" // Flag token.
    );

    opt.parse(argc, argv);

    interactive = opt.isSet("-I");
    opt.get("--lgp")->getInt(lgp);
    live_prep = not opt.get("-F")->isSet;

    opt.resetArgs();
}

void OnlineOptions::finalize(ez::ezOptionParser& opt, int argc,
        const char** argv)
{
    opt.resetArgs();
    opt.parse(argc, argv);

    vector<string*> allArgs(opt.firstArgs);
    allArgs.insert(allArgs.end(), opt.lastArgs.begin(), opt.lastArgs.end());
    string usage;
    vector<string> badOptions;
    unsigned int i;

    if (allArgs.size() != 3u - opt.isSet("-p"))
    {
        cerr << "ERROR: incorrect number of arguments to Player-Online.x\n";
        cerr << "Arguments given were:\n";
        for (unsigned int j = 1; j < allArgs.size(); j++)
            cout << "'" << *allArgs[j] << "'" << endl;
        opt.getUsage(usage);
        cout << usage;
        exit(1);
    }
    else
    {
        if (opt.isSet("-p"))
            opt.get("-p")->getInt(playerno);
        else
            sscanf((*allArgs[1]).c_str(), "%d", &playerno);
        progname = *allArgs[2 - opt.isSet("-p")];
    }

    if (!opt.gotRequired(badOptions))
    {
        for (i = 0; i < badOptions.size(); ++i)
            cerr << "ERROR: Missing required option " << badOptions[i] << ".";
        opt.getUsage(usage);
        cout << usage;
        exit(1);
    }

    if (!opt.gotExpected(badOptions))
    {
        for (i = 0; i < badOptions.size(); ++i)
            cerr << "ERROR: Got unexpected number of arguments for option "
                    << badOptions[i] << ".";
        opt.getUsage(usage);
        cout << usage;
        exit(1);
    }
}
