/*
 * OnlineOptions.cpp
 *
 */

#include "OnlineOptions.h"
#include "Math/gfp.h"

using namespace std;

OnlineOptions OnlineOptions::singleton;

OnlineOptions::OnlineOptions() : playerno(-1)
{
    interactive = false;
    lgp = gfp::MAX_N_BITS;
    live_prep = true;
    batch_size = 10000;
    memtype = "empty";
}

OnlineOptions::OnlineOptions(ez::ezOptionParser& opt, int argc,
        const char** argv, int default_batch_size, bool default_live_prep) :
        OnlineOptions()
{
    if (default_batch_size <= 0)
        default_batch_size = batch_size;

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
    string default_lgp = to_string(lgp);
    opt.add(
          default_lgp.c_str(), // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          ("Bit length of GF(p) field (default: " + default_lgp + ")").c_str(), // Help description.
          "-lgp", // Flag token.
          "--lgp" // Flag token.
    );
    if (default_live_prep)
        opt.add(
                "", // Default.
                0, // Required?
                0, // Number of args expected.
                0, // Delimiter if expecting multiple args.
                "Preprocessing from files", // Help description.
                "-F", // Flag token.
                "--file-preprocessing" // Flag token.
        );
    else
        opt.add(
                "", // Default.
                0, // Required?
                0, // Number of args expected.
                0, // Delimiter if expecting multiple args.
                "Live preprocessing", // Help description.
                "-L", // Flag token.
                "--live-preprocessing" // Flag token.
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

    opt.add(
            to_string(default_batch_size).c_str(), // Default.
            0, // Required?
            1, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            ("Size of preprocessing batches (default: " + to_string(default_batch_size) + ")").c_str(), // Help description.
            "-b", // Flag token.
            "--batch-size" // Flag token.
    );
    opt.add(
            memtype.c_str(), // Default.
            0, // Required?
            1, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Where to obtain memory, new|old|empty (default: empty)\n\t"
            "new: copy from Player-Memory-P<i> file\n\t"
            "old: reuse previous memory in Memory-P<i>\n\t"
            "empty: create new empty memory", // Help description.
            "-m", // Flag token.
            "--memory" // Flag token.
    );

    opt.parse(argc, argv);

    interactive = opt.isSet("-I");
    opt.get("--lgp")->getInt(lgp);
    if (default_live_prep)
        live_prep = not opt.get("-F")->isSet;
    else
        live_prep = opt.get("-L")->isSet;
    opt.get("-b")->getInt(batch_size);
    opt.get("--memory")->getString(memtype);

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
