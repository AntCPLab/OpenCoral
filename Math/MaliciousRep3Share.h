/*
 * MaliciousRep3Share.h
 *
 */

#ifndef MATH_MALICIOUSREP3SHARE_H_
#define MATH_MALICIOUSREP3SHARE_H_

#include "Rep3Share.h"
#include "gfp.h"

template<class T> class HashMaliciousRepMC;
template<class T> class Beaver;

template<class T>
class MaliciousRep3Share : public Rep3Share<T>
{
    typedef Rep3Share<T> super;

public:
    typedef Beaver<MaliciousRep3Share<T>> Protocol;
    typedef HashMaliciousRepMC<MaliciousRep3Share<T>> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ReplicatedInput<MaliciousRep3Share<T>> Input;
    typedef ReplicatedPrivateOutput<MaliciousRep3Share<T>> PrivateOutput;
    typedef Rep3Share<T> Honest;

    static string type_short()
    {
        return "M" + string(1, gfp::type_char());
    }

    MaliciousRep3Share()
    {
    }
    template<class U>
    MaliciousRep3Share(const U& other) : super(other)
    {
    }
};

#endif /* MATH_MALICIOUSREP3SHARE_H_ */
