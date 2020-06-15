#include "Processor/Machine.h"
#include "Processor/OnlineOptions.h"
#include "Math/Setup.h"
#include "Protocols/Share.h"
#include "Tools/ezOptionParser.h"
#include "Networking/Server.h"

#include <iostream>
#include <map>
#include <string>
#include <stdio.h>
using namespace std;

template<class T, class U>
int spdz_main(int argc, const char** argv, ez::ezOptionParser& opt, bool live_prep_default = true)
{
    OnlineOptions& online_opts = OnlineOptions::singleton;
    online_opts = {opt, argc, argv, 1000, live_prep_default, T::clear::invertible};

    opt.example = string() + argv[0] + " -p 0 -N 2 sample-prog\n" + argv[0]
            + " -h localhost -p 1 -N 2 sample-prog\n";

    opt.add(
          to_string(U::clear::default_degree()).c_str(), // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          ("Bit length of GF(2^n) field (default: " + to_string(U::clear::default_degree()) + ")").c_str(), // Help description.
          "-lg2", // Flag token.
          "--lg2" // Flag token.
    );
    opt.add(
          "5000", // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Port number base to attempt to start connections from (default: 5000)", // Help description.
          "-pn", // Flag token.
          "--portnumbase" // Flag token.
    );
    opt.add(
          "", // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Port to listen on (default: port number base + player number)", // Help description.
          "-mp", // Flag token.
          "--my-port" // Flag token.
    );
    opt.add(
          "localhost", // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Host where Server.x or party 0 is running to coordinate startup "
          "(default: localhost). "
          "Ignored if --ip-file-name is used.", // Help description.
          "-h", // Flag token.
          "--hostname" // Flag token.
    );
    opt.add(
      "", // Default.
      0, // Required?
      1, // Number of args expected.
      0, // Delimiter if expecting multiple args.
      "Filename containing list of party ip addresses. Alternative to --hostname and running Server.x for startup coordination.", // Help description.
      "-ip", // Flag token.
      "--ip-file-name" // Flag token.
    );
    opt.add(
          "0", // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Sum at most n shares at once when using indirect communication", // Help description.
          "-s", // Flag token.
          "--opening-sum" // Flag token.
    );
    opt.add(
          "", // Default.
          0, // Required?
          0, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Use player-specific threads for communication", // Help description.
          "-t", // Flag token.
          "--threads" // Flag token.
    );
    opt.add(
          "0", // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Maximum number of parties to send to at once", // Help description.
          "-mb", // Flag token.
          "--max-broadcast" // Flag token.
    );
    opt.add(
          "2", // Default.
          0, // Required?
          1, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Number of players (default: 2). "
          "Ignored if external server is used.", // Help description.
          "-N", // Flag token.
          "--nparties" // Flag token.
    );
    opt.add(
          "", // Default.
          0, // Required?
          0, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Use external server. "
          "Default is to coordinate through player 0.", // Help description.
          "-ext-server", // Flag token.
          "--external-server" // Flag token.
    );
    opt.add(
          "", // Default.
          0, // Required?
          0, // Number of args expected.
          0, // Delimiter if expecting multiple args.
          "Use encrypted channels.", // Help description.
          "-e", // Flag token.
          "--encrypted" // Flag token.
    );

    string hostname, ipFileName;
    int lg2, pnbase, opening_sum, max_broadcast;
    int my_port;

    online_opts.finalize(opt, argc, argv);
    opt.get("--portnumbase")->getInt(pnbase);
    opt.get("--lg2")->getInt(lg2);
    opt.get("--hostname")->getString(hostname);
    opt.get("--ip-file-name")->getString(ipFileName);
    opt.get("--opening-sum")->getInt(opening_sum);
    opt.get("--max-broadcast")->getInt(max_broadcast);

    ez::OptionGroup* mp_opt = opt.get("--my-port");
    if (mp_opt->isSet)
      mp_opt->getInt(my_port);
    else
      my_port = Names::DEFAULT_PORT;

    int mynum = online_opts.playerno;
    int playerno = online_opts.playerno;

    Names playerNames;
    Server* server = 0;
    if (ipFileName.size() > 0) {
      if (my_port != Names::DEFAULT_PORT)
        throw runtime_error("cannot set port number when using IP file");
      playerNames.init(playerno, pnbase, ipFileName);
    } else {
      if (not opt.get("-ext-server")->isSet)
      {
        if (my_port != Names::DEFAULT_PORT)
          throw runtime_error("cannot set port number when not using Server.x");
        int nplayers;
        opt.get("-N")->getInt(nplayers);
        server = Server::start_networking(playerNames, mynum, nplayers,
            hostname, pnbase);
      }
      else
      {
        cerr << "Relying on external Server.x" << endl;
        playerNames.init(playerno, pnbase, my_port, hostname.c_str());
      }
    }
        
#ifndef INSECURE
    try
#endif
    {
        Machine<T, U>(playerno, playerNames, online_opts.progname, online_opts.memtype, lg2,
                online_opts.direct, opening_sum,
                opt.get("--threads")->isSet, max_broadcast,
                opt.get("--encrypted")->isSet, online_opts.live_prep,
                online_opts).run();

        if (server)
          delete server;

#ifdef VERBOSE
        cerr << "Command line:";
        for (int i = 0; i < argc; i++)
            cerr << " " << argv[i];
        cerr << endl;
#endif
    }
#ifndef INSECURE
    catch(...)
    {
        Machine<T, U> machine(playerNames);
        machine.live_prep = false;
        thread_info<T, U>::purge_preprocessing(machine);
        throw;
    }
#endif

    return 0;
}


