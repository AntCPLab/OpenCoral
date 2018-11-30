/*
 * ReplicatedParty.cpp
 *
 */

#include "ReplicatedParty.h"
#include "Thread.h"
#include "MaliciousRepThread.h"
#include "Networking/Server.h"
#include "Tools/ezOptionParser.h"
#include "Tools/benchmarking.h"

namespace GC
{

template<class T>
ReplicatedParty<T>::ReplicatedParty(int argc, const char** argv) :
        ThreadMaster<T>(online_opts), online_opts(opt, argc, argv)
{
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
            "Check opening by communication instead of hashing.", // Help description.
            "-c", // Flag token.
            "--communication" // Flag token.
    );
    opt.parse(argc, argv);
    opt.syntax = "./replicated-bin-party.x [OPTIONS] <progname>";
    if (opt.lastArgs.size() == 1)
    {
        this->progname = *opt.lastArgs[0];
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
    this->machine.use_encryption = not opt.get("-u")->isSet;
    this->machine.more_comm_less_comp = opt.get("-c")->isSet;

    T::out.activate(my_num == 0 or online_opts.interactive);

    if (not this->machine.use_encryption)
        insecure("unencrypted communication");

    Server* server = Server::start_networking(this->N, my_num, 3, hostname, pnb);

    this->run();

    if (server)
        delete server;
}

template<>
Thread<SemiHonestRepSecret>* ReplicatedParty<SemiHonestRepSecret>::new_thread(int i)
{
    return ThreadMaster<SemiHonestRepSecret>::new_thread(i);
}

template<>
Thread<MaliciousRepSecret>* ReplicatedParty<MaliciousRepSecret>::new_thread(int i)
{
    return new MaliciousRepThread(i, *this);
}

template<>
void ReplicatedParty<SemiHonestRepSecret>::post_run()
{
}

template<>
void ReplicatedParty<MaliciousRepSecret>::post_run()
{
    DataPositions usage;
    for (auto thread : threads)
        usage.increase(((MaliciousRepThread*)thread)->usage);
    usage.print_cost();
}

extern template class ReplicatedSecret<SemiHonestRepSecret>;
extern template class ReplicatedSecret<MaliciousRepSecret>;

template class ReplicatedParty<SemiHonestRepSecret>;
template class ReplicatedParty<MaliciousRepSecret>;

}
