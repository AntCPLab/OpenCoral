/*
 * CoralMascotShare.h
 *
 */

#ifndef PROTOCOLS_CORALMASCOTSHARE_H_
#define PROTOCOLS_CORALMASCOTSHARE_H_

#include "Share.h"

template<class T>
class CoralMascotShare : public Share<T>
{
    typedef CoralMascotShare This;
    typedef Share<T> super;

public:
    typedef Share<typename T::next> prep_type;
    typedef This prep_check_type;
    typedef This bit_prep_type;
    typedef This input_check_type;
    typedef This input_type;
    typedef MascotMultiplier<This> Multiplier;
    typedef MascotTripleGenerator<prep_type> TripleGenerator;
    typedef T sacri_type;
    typedef typename T::Square Rectangle;
    typedef Rectangle Square;

    typedef MAC_Check_<This> MAC_Check;
    typedef Direct_MAC_Check<This> Direct_MC;
    typedef ::Input<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;
    typedef CoralGfp<This> Protocol;
    typedef MascotFieldPrep<This> LivePrep;
    typedef MascotPrep<This> RandomPrep;
    typedef MascotTriplePrep<This> TriplePrep;

    static const bool expensive = true;

    typedef GC::RmfeShare bit_type;

    CoralMascotShare()
    {
    }

    template<class U>
    CoralMascotShare(const U& other) :
            super(other)
    {
    }
};


#endif /* PROTOCOLS_CORALMASCOTSHARE_H_ */
