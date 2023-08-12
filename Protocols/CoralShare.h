/*
 * CoralShare.h
 *
 */

#ifndef PROTOCOLS_CORALSHARE_H_
#define PROTOCOLS_CORALSHARE_H_

#include "Math/Z2k.h"
#include "Protocols/Share.h"
#include "Protocols/MAC_Check.h"
#include "Processor/DummyProtocol.h"
#include "OT/Rectangle.h"


// template<int K, int S> class Spdz2kMultiplier;
// template<class T> class Spdz2kTripleGenerator;
template<class T> class Coral;
template<class T> class CoralPrep;

namespace GC
{
class RmfeSecret;
}

template<int K, int S>
class CoralShare : public Spdz2kShare<K, S>
{
public:
    typedef Spdz2kShare<K, S> super;
    using typename super::open_type;

    typedef CoralShare prep_type;
    typedef CoralShare<K + 2, S> bit_prep_type;
    typedef CoralShare<K + S, S> prep_check_type;
    typedef CoralShare input_check_type;
    typedef MAC_Check_Z2k<Z2<K + S>, Z2<S>, open_type, CoralShare> MAC_Check;
    typedef Coral<CoralShare> Protocol;
    typedef CoralPrep<CoralShare> LivePrep;
    typedef ::Input<CoralShare> Input;
    // [zico] this needs to change to HE gen
    typedef Spdz2kTripleGenerator<CoralShare> TripleGenerator;
    typedef Spdz2kMultiplier<CoralShare> Multiplier;

    typedef GC::RmfeShare bit_type;
    

    static string type_string() { return "Coral^(" + to_string(K) + "+" + to_string(S) + ")"; }
    static string type_short() { return "Z" + to_string(K) + "," + to_string(S); }

    CoralShare() {}
    template<class T, class V>
    CoralShare(const Share_<T, V>& x) : super(x) {}
    template<class T, class V>
    CoralShare(const T& share, const V& mac) : super(share, mac) {}
};


template<int K, int S>
CoralShare<K, S> operator*(const typename CoralShare<K,S>::open_type& x, CoralShare<K, S>& y)
{
    return typename CoralShare<K,S>::tmp_type(x) * typename CoralShare<K,S>::super(y);
}

#endif /* PROTOCOLS_CORALSHARE_H_ */
