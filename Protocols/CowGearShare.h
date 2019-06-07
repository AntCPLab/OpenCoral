/*
 * CowGearShare.h
 *
 */

#ifndef PROTOCOLS_COWGEARSHARE_H_
#define PROTOCOLS_COWGEARSHARE_H_

#include "Share.h"

template<class T> class CowGearPrep;

template<class T>
class CowGearShare : public Share<T>
{
    typedef CowGearShare This;
    typedef Share<T> super;

public:
    typedef MAC_Check_<This> MAC_Check;
    typedef Direct_MAC_Check<This> Direct_MC;
    typedef ::Input<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;
    typedef SPDZ<This> Protocol;
    typedef CowGearPrep<This> LivePrep;

    const static bool needs_ot = false;

    CowGearShare()
    {
    }
    template<class U>
    CowGearShare(const U& other) :
            super(other)
    {
    }

};

#endif /* PROTOCOLS_COWGEARSHARE_H_ */
