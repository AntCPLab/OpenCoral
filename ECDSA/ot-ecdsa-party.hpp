/*
 * fake-spdz-ecdsa-party.cpp
 *
 */

#include "Networking/Server.h"
#include "Networking/CryptoPlayer.h"
#include "Math/gfp.h"
#include "ECDSA/P256Element.h"
#include "Protocols/SemiShare.h"
#include "Processor/BaseMachine.h"

#include "ECDSA/preprocessing.hpp"
#include "ECDSA/sign.hpp"
#include "Protocols/Beaver.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Protocols/MascotPrep.hpp"
#include "Processor/Processor.hpp"
#include "Processor/Data_Files.hpp"
#include "Processor/Input.hpp"

#include <assert.h>

template<template<class U> class T>
void run(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Delay multiplication until signing", // Help description.
            "-D", // Flag token.
            "--delay-multiplication" // Flag token.
    );
    Names N(opt, argc, argv, 2);
    int n_tuples = 1000;
    if (not opt.lastArgs.empty())
        n_tuples = atoi(opt.lastArgs[0]->c_str());
    PlainPlayer P(N);
    P256Element::init();
    gfp1::init_field(P256Element::Scalar::pr(), false);

    BaseMachine machine;
    machine.ot_setups.resize(1);
    for (int i = 0; i < 2; i++)
        machine.ot_setups[0].push_back({P, true});

    P256Element::Scalar keyp;
    SeededPRNG G;
    keyp.randomize(G);

    typedef T<P256Element::Scalar> pShare;
    DataPositions usage;

    OnlineOptions::singleton.batch_size = 1;
    typename pShare::Direct_MC MCp(keyp, N, 0);
    ArithmeticProcessor _({}, 0);
    typename pShare::LivePrep sk_prep(0, usage);
    SubProcessor<pShare> sk_proc(_, MCp, sk_prep, P);
    pShare sk, __;
    // synchronize
    Bundle<octetStream> bundle(P);
    P.Broadcast_Receive(bundle, false);
    Timer timer;
    timer.start();
    sk_prep.get_two(DATA_INVERSE, sk, __);
    cout << "Secret key generation took " << timer.elapsed() * 1e3 << " ms" << endl;

    OnlineOptions::singleton.batch_size = n_tuples;
    typename pShare::LivePrep prep(0, usage);
    SubProcessor<pShare> proc(_, MCp, prep, P);

    bool prep_mul = not opt.isSet("-D");
    vector<EcTuple<T>> tuples;
    preprocessing(tuples, n_tuples, sk, proc, prep_mul);
    //check(tuples, sk, keyp, P);
    sign_benchmark(tuples, sk, MCp, P, prep_mul ? 0 : &proc);
}
