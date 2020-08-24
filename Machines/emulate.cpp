/*
 * emulate.cpp
 *
 */

#include "Protocols/FakeShare.h"
#include "Processor/Machine.h"
#include "Math/Z2k.h"
#include "Math/gf2n.h"
#include "Processor/RingOptions.h"

#include "Processor/Machine.hpp"
#include "Math/Z2k.hpp"
#include "Protocols/Replicated.hpp"
#include "Protocols/ShuffleSacrifice.hpp"
#include "Protocols/ReplicatedPrep.hpp"
#include "Protocols/FakeShare.hpp"

SwitchableOutput GC::NoShare::out;

int main(int argc, const char** argv)
{
    assert(argc > 1);
    OnlineOptions online_opts;
    Names N(0, 9999, vector<string>({"localhost"}));
    ez::ezOptionParser opt;
    RingOptions ring_opts(opt, argc, argv);
    opt.parse(argc, argv);
    string progname;
    if (opt.firstArgs.size() > 1)
        progname = *opt.firstArgs.at(1);
    else if (not opt.lastArgs.empty())
        progname = *opt.lastArgs.at(0);
    else if (not opt.unknownArgs.empty())
        progname = *opt.unknownArgs.at(0);
    else
    {
        string usage;
        opt.getUsage(usage);
        cerr << usage << endl;
        exit(1);
    }

    switch (ring_opts.R)
    {
    case 64:
        Machine<FakeShare<SignedZ2<64>>, FakeShare<gf2n>>(0, N, progname,
                online_opts.memtype, gf2n::default_degree(), 0, 0, 0, 0, true,
                online_opts.live_prep, online_opts).run();
        break;
    case 128:
        Machine<FakeShare<SignedZ2<128>>, FakeShare<gf2n>>(0, N, progname,
                online_opts.memtype, gf2n::default_degree(), 0, 0, 0, 0, true,
                online_opts.live_prep, online_opts).run();
        break;
    default:
        cerr << "Not compiled for " << ring_opts.R << "-bit rings" << endl;
    }
}
