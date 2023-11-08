/*
 * CoralLowGearShare.h
 *
 */

#ifndef PROTOCOLS_CORALLOWGEARSHARE_H_
#define PROTOCOLS_CORALLOWGEARSHARE_H_

#include "LowGearShare.h"

template<class T>
class CoralLowGearShare : public LowGearShare<T>
{
    typedef CoralLowGearShare This;
    typedef LowGearShare<T> super;

public:
    typedef MAC_Check_<This> MAC_Check;
    typedef Direct_MAC_Check<This> Direct_MC;
    typedef ::Input<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;
    typedef CoralGfp<This> Protocol;
    typedef CowGearPrep<This> LivePrep;

    typedef GC::RmfeShare bit_type;

    const static false_type covert;

    CoralLowGearShare()
    {
    }

    template<class U>
    CoralLowGearShare(const U& other) :
            super(other)
    {
    }
};

template<class T>
const false_type CoralLowGearShare<T>::covert;

#endif /* PROTOCOLS_CORALLOWGEARSHARE_H_ */
