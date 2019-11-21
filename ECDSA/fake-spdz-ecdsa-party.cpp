/*
 * fake-spdz-ecdsa-party.cpp
 *
 */

#include "Networking/Server.h"
#include "Networking/CryptoPlayer.h"
#include "Math/gfp.h"
#include "ECDSA/P256Element.h"

#include "ECDSA/preprocessing.hpp"
#include "ECDSA/sign.hpp"
#include "Protocols/Beaver.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Protocols/Share.hpp"
#include "Protocols/MAC_Check.hpp"
#include "Processor/Input.hpp"
#include "Processor/Processor.hpp"
#include "Processor/Data_Files.hpp"

#include <assert.h>

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    EcdsaOptions opts(opt, argc, argv);
    Names N(opt, argc, argv, 2);
    int n_tuples = 1000;
    if (not opt.lastArgs.empty())
        n_tuples = atoi(opt.lastArgs[0]->c_str());
    PlainPlayer P(N);
    P256Element::init();

    P256Element::Scalar keyp;
    gf2n key2;
    string prefix = PREP_DIR "ECDSA/";
    read_mac_keys(prefix, N, keyp, key2);

    typedef Share<P256Element::Scalar> pShare;
    DataPositions usage;
    Sub_Data_Files<pShare> prep(N, prefix, usage);
    typename pShare::Direct_MC MCp(keyp);
    ArithmeticProcessor _({}, 0);
    SubProcessor<pShare> proc(_, MCp, prep, P);

    pShare sk, __;
    proc.DataF.get_two(DATA_INVERSE, sk, __);

    vector<EcTuple<Share>> tuples;
    preprocessing(tuples, n_tuples, sk, proc, opts);
    check(tuples, sk, keyp, P);
    sign_benchmark(tuples, sk, MCp, P, opts);
}
