/*
 * ReplicatedMachine.cpp
 *
 */

#include "Tools/ezOptionParser.h"
#include "Tools/benchmarking.h"
#include "Networking/Server.h"
#include "Math/Rep3Share.h"
#include "Processor/Machine.h"
#include "ReplicatedMachine.h"

template<class T, class U>
ReplicatedMachine<T, U>::ReplicatedMachine(int argc, const char** argv,
          string name, ez::ezOptionParser& opt, int nplayers)
{
    OnlineOptions online_opts(opt, argc, argv);
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
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Unencrypted communication.", // Help description.
            "-u", // Flag token.
            "--unencrypted" // Flag token.
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
    opt.syntax = "./" + name + "-party.x [OPTIONS] <playerno> <progname>";
    opt.resetArgs();
    opt.parse(argc, argv);
    vector<string*> allArgs(opt.firstArgs);
    allArgs.insert(allArgs.end(), opt.lastArgs.begin(), opt.lastArgs.end());

    int playerno;
    string progname;

    if (allArgs.size() != 3)
    {
        cerr << "ERROR: incorrect number of arguments to " << argv[0] << endl;
        cerr << "Arguments given were:\n";
        for (unsigned int j = 1; j < allArgs.size(); j++)
            cout << "'" << *allArgs[j] << "'" << endl;
        string usage;
        opt.getUsage(usage);
        cout << usage;
        exit(1);
    }
    else
    {
        playerno = atoi(allArgs[1]->c_str());
        progname = *allArgs[2];

    }

    int pnb;
    string hostname;
    opt.get("-pn")->getInt(pnb);
    opt.get("-h")->getString(hostname);
    bool use_encryption = not opt.get("-u")->isSet;
    bool live_prep = not opt.get("-F")->isSet;

    if (not use_encryption)
        insecure("unencrypted communication");
    Names N;
    Server* server = Server::start_networking(N, playerno, nplayers, hostname, pnb);

    Machine<T, U>(playerno, N, progname, "empty",
            gf2n::default_degree(), 0, 0, 0, 0, 0, use_encryption,
            live_prep, online_opts).run();

    if (server)
        delete server;
}
