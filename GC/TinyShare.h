/*
 * TinyShare.h
 *
 */

#ifndef GC_TINYSHARE_H_
#define GC_TINYSHARE_H_

#include "ShareSecret.h"
#include "ShareParty.h"
#include "Secret.h"
#include "Protocols/Spdz2kShare.h"


namespace GC
{

template<int S> class TinySecret;
template<class T> class ShareThread;
template<class T> class TinierSharePrep;

template<int S>
class TinyShare : public Spdz2kShare<1, S>, public ShareSecret<TinySecret<S>>
{
    typedef TinyShare This;

public:
    typedef Spdz2kShare<1, S> super;

    typedef void DynamicMemory;

    typedef Beaver<This> Protocol;
    typedef MAC_Check_Z2k_<This> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef MAC_Check MC;
    typedef ::Input<This> Input;
    typedef TinierSharePrep<This> LivePrep;

    typedef SwitchableOutput out_type;

    typedef This small_type;

    typedef NoShare bit_type;

    // `is_real` exists in multiple base classes (ShareSecret and ShareInterface)
    static const bool is_real = true;
    static const false_type tight_packed;
    static const bool is_bit_type = true;

    static string name()
    {
        return "tiny share";
    }

    TinyShare()
    {
    }
    TinyShare(const typename super::super::super& other) :
            super(other)
    {
    }
    TinyShare(const super& other) :
            super(other)
    {
    }
    TinyShare(const TinySecret<S>& other);

    void XOR(const This& a, const This& b)
    {
        *this = a + b;
    }

    void public_input(bool input)
    {
        auto& party = this->get_party();
        *this = super::constant(input, party.P->my_num(),
                party.MC->get_alphai());
    }

    static MAC_Check* new_mc(typename super::mac_key_type mac_key)
    {
        return new MAC_Check(mac_key);
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

template<int S>
const false_type TinyShare<S>::tight_packed;

} /* namespace GC */

#endif /* GC_TINYSHARE_H_ */
