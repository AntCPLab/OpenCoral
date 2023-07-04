/*
 * RmfeShare.h
 *
 */

#ifndef GC_RMFESHARE_H_
#define GC_RMFESHARE_H_

#include "Processor/InsecureProtocol.h"
#include "Protocols/Share.h"
#include "Math/Bit.h"
#include "Protocols/RmfeBeaver.h"

class gf2n_mac_key : public gf2n_short
{
public:
    gf2n_mac_key()
    {
    }

    template<class T>
    gf2n_mac_key(const T& other) :
            gf2n_short(other)
    {
    }
};

namespace GC
{

template<class T, class V> class RmfeSecret;
// template<class T> class TinierSharePrep;

template<class T, class V>
class RmfeShare: public Share_<SemiShare<T>, SemiShare<V>>,
        public ShareSecret<RmfeSecret<T, V>>
{
    typedef RmfeShare This;

public:
    typedef Share_<SemiShare<T>, SemiShare<V>> super;

    typedef V mac_key_type;
    typedef V mac_type;
    typedef V sacri_type;
    typedef Share<V> input_check_type;
    typedef This prep_type;
    typedef This prep_check_type;
    typedef This bit_prep_type;
    typedef This bit_type;

    typedef InsecureMC<This> MAC_Check;
    typedef InsecureLivePrep2PC<This> LivePrep;
    typedef ::Input<This> Input;
    typedef RmfeBeaver<This> Protocol;
    // typedef NPartyTripleGenerator<TinierSecret<T>> TripleGenerator;

    typedef void DynamicMemory;
    typedef SwitchableOutput out_type;

    typedef This part_type;
    typedef This small_type;

    // typedef TinierSecret<T> whole_type;

    typedef InsecureMC<This> MC;

    static const int default_length = 1;

    static string name()
    {
        return "rmfe share";
    }

    static string type_string()
    {
        return "Rmfe";
    }

    static string type_short()
    {
        return "Rf";
    }

    // static ShareThread<TinierSecret<T>>& get_party()
    // {
    //     return ShareThread<TinierSecret<T>>::s();
    // }

    static ShareThread<This>& get_party()
    {
        return ShareThread<This>::s();
    }

    static MAC_Check* new_mc(mac_key_type mac_key)
    {
        return new MAC_Check(mac_key);
    }

    RmfeShare()
    {
    }
    RmfeShare(const super& other) :
            super(other)
    {
    }
    RmfeShare(const typename super::share_type& share, const typename super::mac_type& mac) :
            super(share, mac)
    {
    }
    RmfeShare(const RmfeSecret<T, V>& other);

    void XOR(const This& a, const This& b)
    {
        *this = a + b;
    }

    This operator^(const This& other) const
    {
        return *this + other;
    }

    This& operator^=(const This& other)
    {
        *this += other;
        return *this;
    }

    void public_input(bool input)
    {
        auto& party = get_party();
        *this = super::constant(input, party.P->my_num(),
                party.MC->get_alphai());
    }

    This lsb() const
    {
        return *this;
    }

    This get_bit(int i)
    {
        assert(i == 0);
        return lsb();
    }
};

} /* namespace GC */

#endif /* GC_RMFEHARE_H_ */
