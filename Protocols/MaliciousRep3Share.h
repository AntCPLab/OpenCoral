/*
 * MaliciousRep3Share.h
 *
 */

#ifndef PROTOCOLS_MALICIOUSREP3SHARE_H_
#define PROTOCOLS_MALICIOUSREP3SHARE_H_

#include "Rep3Share.h"

template<class T> class HashMaliciousRepMC;
template<class T> class Beaver;
template<class T> class MaliciousRepPrepWithBits;

namespace GC
{
class MaliciousRepSecret;
}

template<class T>
class MaliciousRep3Share : public Rep3Share<T>
{
    typedef Rep3Share<T> super;

public:
    typedef Beaver<MaliciousRep3Share<T>> Protocol;
    typedef HashMaliciousRepMC<MaliciousRep3Share<T>> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ReplicatedInput<MaliciousRep3Share<T>> Input;
    typedef ::PrivateOutput<MaliciousRep3Share<T>> PrivateOutput;
    typedef Rep3Share<T> Honest;
    typedef MaliciousRepPrepWithBits<MaliciousRep3Share> LivePrep;
    typedef MaliciousRep3Share prep_type;
    typedef T random_type;

    typedef GC::MaliciousRepSecret bit_type;

    const static bool expensive = true;

    static string type_short()
    {
        return "M" + string(1, T::type_char());
    }

    MaliciousRep3Share()
    {
    }
    MaliciousRep3Share(const T& other, int my_num, T alphai = {}) :
            super(other, my_num, alphai)
    {
    }
    template<class U>
    MaliciousRep3Share(const U& other) : super(other)
    {
    }
};

#endif /* PROTOCOLS_MALICIOUSREP3SHARE_H_ */
