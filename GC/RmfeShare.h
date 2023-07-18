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
#include "RmfeInput.h"
#include "Tools/mpdz_ntl_types.h"

class gf2n_rmfe;

class bitvec_rmfe : public BitVec
{
    typedef BitVec super;
public:
    static const int DEFAULT_LENGTH = 12;

    bitvec_rmfe() {
        
    }

    // NOTE: This constructor includes decoding.
    bitvec_rmfe(const gf2n_rmfe& encoded) {
        NTL::GF2X ntl_tmp;
        BitVec decoded;
        conv(ntl_tmp, encoded);
        conv(decoded, Gf2RMFE::s().decode(ntl_tmp));
        super::operator=(decoded);
    }
};

class gf2n_rmfe : public gf2n_short
{
    typedef gf2n_short super;
public:
    gf2n_rmfe()
    {
    }

    // NOTE: This constructor includes encoding.
    gf2n_rmfe(const BitVec& decoded) {
        NTL::vec_GF2 ntl_tmp;
        conv(ntl_tmp, decoded, bitvec_rmfe::DEFAULT_LENGTH);
        conv(*this, Gf2RMFE::s().encode(ntl_tmp));
    }

    static const int DEFAULT_LENGTH = 48;

    static int length()         { return n == 0 ? DEFAULT_LENGTH : n; }
    static int default_degree() { return DEFAULT_LENGTH; }

    static void init_field(int nn = 0) {
        super::init_field(nn == 0 ? DEFAULT_LENGTH : nn);
    }

    template<class T>
    gf2n_rmfe(const T& other) :
            gf2n_short(other)
    {
    }
};


namespace GC
{

template<class T> class RmfeSecret;

template<class T>
class RmfeShare: public Share_<SemiShare<T>, SemiShare<T>>,
        public ShareSecret<RmfeSecret<T>>
{
    typedef RmfeShare This;

public:
    typedef Share_<SemiShare<T>, SemiShare<T>> super;

    typedef T open_type;
    typedef T clear;
    typedef bitvec_rmfe raw_type;

    typedef T mac_key_type;
    typedef T mac_type;
    typedef T sacri_type;
    typedef Share<T> input_check_type;
    typedef This prep_type;
    typedef This prep_check_type;
    typedef This bit_prep_type;
    typedef This bit_type;

    typedef InsecureMC<This> MAC_Check;
    typedef InsecureLivePrep2PC<This> LivePrep;
    typedef RmfeInput<This> Input;
    typedef RmfeBeaver<This> Protocol;
    // typedef NPartyTripleGenerator<TinierSecret<T>> TripleGenerator;

    typedef void DynamicMemory;
    typedef SwitchableOutput out_type;

    typedef This part_type;
    typedef This small_type;

    // typedef TinierSecret<T> whole_type;

    typedef InsecureMC<This> MC;

    static const int default_length = bitvec_rmfe::DEFAULT_LENGTH;

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
    RmfeShare(const RmfeSecret<T>& other);

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

    BitVec decoded_share() const {
        NTL::GF2X ntl_tmp;
        BitVec decoded;
        conv(ntl_tmp, this->get_share());
        conv(decoded, Gf2RMFE::s().decode(ntl_tmp));
        return decoded;
    }
};

} /* namespace GC */

#endif /* GC_RMFEHARE_H_ */
