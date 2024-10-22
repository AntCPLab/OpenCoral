/*
 * coral-party.cpp
 *
 */

// // [zico] need to update
// #define NO_SECURITY_CHECK
// #define VERBOSE_DEBUG_PRINT
// #define RMFE_UNIT
// #define CHECK_SIZE

#include "Processor/Machine.h"
#include "Processor/RingOptions.h"
#include "Protocols/CoralShare.h"
#include "Math/gf2n.h"
#include "Networking/Server.h"

#include "Processor/RingMachine.hpp"
#include "Math/Z2k.hpp"

#include "Coral.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    opt.add(
        "64", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Coral security parameter (default: 64)", // Help description.
        "-SP", // Flag token.
        "--coral-security" // Flag token.
    );
    opt.parse(argc, argv);
    int s;
    opt.get("-SP")->getInt(s);
    opt.resetArgs();

    RingOptions ring_options(opt, argc, argv);
    OnlineOptions& online_opts = OnlineOptions::singleton;
    online_opts = {opt, argc, argv, CoralShare<64, 64>(), true};
    DishonestMajorityMachine machine(argc, argv, opt, online_opts, gf2n());
    // Explicitly set the GF(2^n) initialization degree for Coral, regardless of any command option.
    machine.set_lg2(RmfeShare::open_type::default_degree());

    int k = ring_options.ring_size_from_opts_or_schedule(online_opts.progname);

#ifdef VERBOSE
    cerr << "Using Coral with ring length " << k << " and security parameter "
            << s << endl;
#endif

#undef Z
#define Z(K, S) \
    if (s == S and k == K) \
        return machine.run<CoralShare<K, S>, Share<gf2n>>();

    Z(64, 64)
    Z(64, 48)
    Z(72, 64)
    Z(72, 48)

#ifdef RING_SIZE
    Z(RING_SIZE, CORAL_DEFAULT_SECURITY)
#endif

    else
    {
        if (s == CORAL_DEFAULT_SECURITY)
        {
            ring_domain_error(k);
        }
        else
        {
            cerr << "not compiled for k=" << k << " and s=" << s << "," << endl;
            cerr << "add Z(" << k << ", " << s << ") to " << __FILE__ << " at line "
                    << (__LINE__ - 11) << " and create Machines/SPDZ2^" << k << "+"
                    << s << ".cpp based on Machines/SPDZ2^72+64.cpp" << endl;
            cerr << "Alternatively, compile with -DRING_SIZE=" << k
                    << " and -DCORAL_DEFAULT_SECURITY=" << s << endl;
        }
        exit(1);
    }
}
