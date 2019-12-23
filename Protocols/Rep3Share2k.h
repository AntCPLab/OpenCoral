/*
 * Rep3Share2k.h
 *
 */

#ifndef PROTOCOLS_REP3SHARE2K_H_
#define PROTOCOLS_REP3SHARE2K_H_

#include "Rep3Share.h"
#include "Math/Z2k.h"

template<class T> class ReplicatedPrep2k;

template<int K>
class Rep3Share2 : public Rep3Share<SignedZ2<K>>
{
    typedef SignedZ2<K> T;

public:
    typedef Replicated<Rep3Share2> Protocol;
    typedef ReplicatedMC<Rep3Share2> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ReplicatedInput<Rep3Share2> Input;
    typedef ::PrivateOutput<Rep3Share2> PrivateOutput;
    typedef ReplicatedPrep2k<Rep3Share2> LivePrep;
    typedef Rep3Share2 Honest;

    typedef GC::SemiHonestRepSecret bit_type;

    Rep3Share2()
    {
    }
    template<class U>
    Rep3Share2(const FixedVec<U, 2>& other)
    {
        FixedVec<T, 2>::operator=(other);
    }
};

#endif /* PROTOCOLS_REP3SHARE2K_H_ */
