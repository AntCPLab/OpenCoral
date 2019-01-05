/*
 * MaliciousShamirShare.h
 *
 */

#ifndef MATH_MALICIOUSSHAMIRSHARE_H_
#define MATH_MALICIOUSSHAMIRSHARE_H_

#include "ShamirShare.h"
#include "Processor/Beaver.h"
#include "Auth/MaliciousShamirMC.h"

template<class T>
class MaliciousShamirShare : public ShamirShare<T>
{
    typedef ShamirShare<T> super;

public:
    typedef Beaver<MaliciousShamirShare<T>> Protocol;
    typedef MaliciousShamirMC<MaliciousShamirShare> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ShamirInput<MaliciousShamirShare> Input;
    typedef ReplicatedPrivateOutput<MaliciousShamirShare> PrivateOutput;
    typedef ShamirShare<T> Honest;

    static string type_short()
    {
        return "M" + super::type_short();
    }

    MaliciousShamirShare()
    {
    }
    template<class U>
    MaliciousShamirShare(const U& other) : super(other)
    {
    }
};

#endif /* MATH_MALICIOUSSHAMIRSHARE_H_ */
