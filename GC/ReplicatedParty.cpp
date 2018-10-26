/*
 * ReplicatedParty.cpp
 *
 */

#include "ReplicatedParty.h"
#include "Thread.h"
#include "Networking/Server.h"
#include "Tools/ezOptionParser.h"
#include "Tools/benchmarking.h"

namespace GC
{

ReplicatedParty::ReplicatedParty(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    opt.add(
            "", // Default.
            1, // Required?
            1, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "This player's number (required)", // Help description.
            "-p", // Flag token.
            "--player" // Flag token.
    );
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
    opt.syntax = "./replicated-bin-party.x [OPTIONS] <progname>";
    if (opt.lastArgs.size() == 1)
    {
        progname = *opt.lastArgs[0];
    }
    else
    {
        string usage;
        opt.getUsage(usage);
        cerr << usage;
        exit(1);
    }

    int my_num;
    int pnb;
    string hostname;
    opt.get("-p")->getInt(my_num);
    opt.get("-pn")->getInt(pnb);
    opt.get("-h")->getString(hostname);

    if (my_num != 0)
        ReplicatedSecret::out.activate(false);

    insecure("unencrypted communication");
    Server* server = Server::start_networking(N, my_num, 3, hostname, pnb);

    run();

    if (server)
        delete server;
}

}
