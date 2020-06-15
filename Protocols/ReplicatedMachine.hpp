/*
 * ReplicatedMachine.cpp
 *
 */

#include "Tools/ezOptionParser.h"
#include "Tools/benchmarking.h"
#include "Tools/NetworkOptions.h"
#include "Networking/Server.h"
#include "Protocols/Rep3Share.h"
#include "Processor/Machine.h"
#include "ReplicatedMachine.h"

template<class T, class U>
ReplicatedMachine<T, U>::ReplicatedMachine(int argc, const char** argv,
          string name, ez::ezOptionParser& opt, int nplayers)
{
    (void) name;

    OnlineOptions online_opts(opt, argc, argv, 10000, true, T::clear::invertible);
    OnlineOptions::singleton = online_opts;
    NetworkOptionsWithNumber network_opts(opt, argc, argv, nplayers, false);
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Unencrypted communication.", // Help description.
            "-u", // Flag token.
            "--unencrypted" // Flag token.
    );
    online_opts.finalize(opt, argc, argv);

    int playerno = online_opts.playerno;
    string progname = online_opts.progname;

    bool use_encryption = not opt.get("-u")->isSet;

    if (not use_encryption)
        insecure("unencrypted communication");
    Names N;
    Server* server = network_opts.start_networking(N, playerno);

    Machine<T, U>(playerno, N, progname, online_opts.memtype,
            gf2n::default_degree(), 0, 0, 0, 0, use_encryption,
            online_opts.live_prep, online_opts).run();

    if (server)
        delete server;
}
