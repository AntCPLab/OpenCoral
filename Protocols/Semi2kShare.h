/*
 * Semi2kShare.h
 *
 */

#ifndef PROTOCOLS_SEMI2KSHARE_H_
#define PROTOCOLS_SEMI2KSHARE_H_

#include "SemiShare.h"
#include "OT/Rectangle.h"
#include "GC/SemiSecret.h"

template<class T> class SemiPrep2k;

template <int K>
class Semi2kShare : public SemiShare<SignedZ2<K>>
{
    typedef SignedZ2<K> T;

public:
    typedef Z2<64> mac_key_type;

    typedef SemiMC<Semi2kShare> MAC_Check;
    typedef DirectSemiMC<Semi2kShare> Direct_MC;
    typedef SemiInput<Semi2kShare> Input;
    typedef ::PrivateOutput<Semi2kShare> PrivateOutput;
    typedef SPDZ<Semi2kShare> Protocol;
    typedef SemiPrep2k<Semi2kShare> LivePrep;

    typedef Semi2kShare prep_type;
    typedef SemiMultiplier<Semi2kShare> Multiplier;
    typedef OTTripleGenerator<prep_type> TripleGenerator;
    typedef Z2kSquare<K> Rectangle;

    typedef GC::SemiSecret bit_type;

    Semi2kShare()
    {
    }
    template<class U>
    Semi2kShare(const U& other) : SemiShare<SignedZ2<K>>(other)
    {
    }
    Semi2kShare(const T& other, int my_num, const T& alphai = {})
    {
        (void) alphai;
        assign(other, my_num);
    }
};

#endif /* PROTOCOLS_SEMI2KSHARE_H_ */
