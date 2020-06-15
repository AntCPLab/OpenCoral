/*
 * MamaShare.h
 *
 */

#ifndef PROTOCOLS_MAMASHARE_H_
#define PROTOCOLS_MAMASHARE_H_

#include "Share.h"
#include "Math/gfp.h"
#include "Math/FixedVec.h"
#include "OT/MamaRectangle.h"

template<class T> class MamaPrep;
template<class T> class MamaMultiplier;
template<class T> class SimpleMascotTripleGenerator;

template<class T, int N>
class MamaShare : public Share_<SemiShare<T>, FixedVec<SemiShare<T>, N>>
{
    typedef MamaShare This;

public:
    typedef FixedVec<SemiShare<T>, N> mac_key_type;
    typedef Share_<SemiShare<T>, mac_key_type> super;

    typedef Beaver<This> Protocol;
    typedef MAC_Check_<This> MAC_Check;
    typedef Direct_MAC_Check<This> Direct_MC;
    typedef ::Input<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;

    typedef MamaPrep<This> LivePrep;
    typedef MamaShare<typename T::next, N> prep_type;
    typedef SimpleMascotTripleGenerator<prep_type> TripleGenerator;
    typedef MascotMultiplier<This> Multiplier;
    typedef FixedVec<T, N> sacri_type;
    typedef This input_type;
    typedef MamaRectangle<T, N> Square;
    typedef typename T::Square Rectangle;

    static const int N_MACS = N;

    static const bool expensive = true;

    static string type_string()
    {
        return "Mama" + to_string(N);
    }

    static void read_or_generate_mac_key(string, Names&, mac_key_type& key)
    {
        SeededPRNG G;
        key.randomize(G);
    }

    MamaShare()
    {
    }

    MamaShare(const super& other) :
        super(other)
    {
    }

    template<class U>
    MamaShare(const MamaShare<U, N>& other) :
        super(other.get_share(), other.get_mac())
    {
    }
};

#endif /* PROTOCOLS_MAMASHARE_H_ */
