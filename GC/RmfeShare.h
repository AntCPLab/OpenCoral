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
#include "RmfeMC.h"
#include "RmfeMultiplier.h"
#include "Tools/mpdz_ntl_types.h"
#include "Tools/Exceptions.h"
#include "Math/mfe.h"
#include "Tools/debug.h"

class gf2n_rmfe;
class bitvec_mfe;

class bitvec_rmfe : public BitVec
{
    typedef BitVec super;
public:
    static const int DEFAULT_LENGTH = 14;

    bitvec_rmfe() {
    }

    bitvec_rmfe(long a): bitvec_rmfe(BitVec(a)) {
        *this = this->mask(DEFAULT_LENGTH);
    }

    bitvec_rmfe(bool a): bitvec_rmfe(BitVec(a)) {}

    bitvec_rmfe(const BitVec& a) : super(a) {}

    bitvec_rmfe(const BitVec::super& a) : super(a) {}

    // NOTE: This constructor includes decoding.
    bitvec_rmfe(const gf2n_rmfe& encoded);

    void randomize(PRNG& G) { 
        super::randomize(G, DEFAULT_LENGTH); 
    }
};

class gf2n_rmfe : public gf2n_short
{
    typedef gf2n_short super;
public:
    typedef ::Square<gf2n_rmfe> Square;
    typedef gf2n_rmfe Scalar;

    gf2n_rmfe()
    {
    }

    // NOTE: This constructor includes rmfe encoding.
    gf2n_rmfe(const bitvec_rmfe& decoded);

    // NOTE: This constructor includes mfe decoding.
    gf2n_rmfe(const bitvec_mfe& encoded);

    gf2n_rmfe(const gf2n_<word>& x): gf2n_short(x) {}

    // /**
    //  * Only exists for compatibility with other parts of the code
    // */
    // explicit gf2n_rmfe(bool x) {
    //     if (!x)
    //         this->a = 0;
    //     else
    //         throw runtime_error("Pass in x=true, but we can only convert 'false' to gf2n_rmfe. Given 'true', \
    //             it is unclear what the caller wants because gf2n_rmfe is an encoded type \
    //             and 1 in bitvec_rmfe does not correspond to 1 in gf2n_rmfe.");
    // }

    /**
     * Only exists for compatibility with other parts of the code
    */
    explicit gf2n_rmfe(long x) {
        if (x == 0)
            this->a = 0;
        else
            throw runtime_error("Pass in x=" + to_string(x) + ", but we can only convert 0 to gf2n_rmfe. Given other values, \
                it is unclear what the caller wants because gf2n_rmfe is an encoded type.");
    }

    explicit gf2n_rmfe(int128 x): gf2n_short(x) {}

    /**
     * Easily resulting in bug, because many types implicitly convertible to 
     * gf2n_rmfe, including those that are not expected, such as BitVec, which is used as the clear type
     * for RmfeShare and should be encoded, but not converted directly. This bug actually happens for the 
     * 'operator&' function when we did not use 'explicit' before. Also all the +-* operators between
     * bitvec_rmfe types actually return BitVec type but not bitvec_rmfe type.
    */
    // template<class T>
    // explicit gf2n_rmfe(const T& other) :
    //         gf2n_short(other)
    // {
    // }

    static const int DEFAULT_LENGTH = 42;

    static int length()         { return n == 0 ? DEFAULT_LENGTH : n; }
    static int default_degree() { return DEFAULT_LENGTH; }

    static void init_field(int nn = 0) {
        (void) nn;
        super::init_field(DEFAULT_LENGTH);
    }

    /**
     * Rewriting this func from gf2n_ because some code
     * directly call this `add` that actually includes an unpacking operation.
    */
    void add(octetStream& os, int n = -1);
    
    /**
	 * Append to buffer in native format.
	 * @param o buffer
	 * @param n (unused)
	 */
    void pack(octetStream& o, int n = -1) const;
    /**
	 * Read from buffer in native format.
	 * @param o buffer
	 * @param n (unused)
	 */
    void unpack(octetStream& o, int n = -1);

    static gf2n_rmfe tau(const gf2n_rmfe& x);
    bool is_normal();

    static gf2n_rmfe random_preimage(const bitvec_rmfe& x);
};

class bitvec_mfe : public BitVector
{
    typedef BitVector super;
public:
    static const int DEFAULT_LENGTH = 225;

    bitvec_mfe(): super(DEFAULT_LENGTH) {
    }

    // NOTE: This constructor includes encoding.
    bitvec_mfe(const gf2n_rmfe& decoded);

    void randomize(PRNG& G) { 
        super::randomize(G); 
    }
};


namespace GC
{

template<class T> class RmfeSharePrep;

class RmfeShare: public Share_<SemiShare<gf2n_rmfe>, SemiShare<gf2n_rmfe>>,
        public ShareSecret<RmfeShare>
{
    typedef RmfeShare This;

public:
    typedef gf2n_rmfe T;
    typedef Share_<SemiShare<T>, SemiShare<T>> super;

    typedef This whole_type;
    typedef This part_type;
    typedef This small_type;

    typedef T open_type;
    // typedef T clear;
    typedef bitvec_rmfe clear;
    typedef bitvec_rmfe raw_type;
    typedef bitvec_mfe encoded_mac_type;

    typedef T mac_key_type;
    typedef T mac_type;
    typedef T sacri_type;
    typedef Share<T> input_check_type;
    typedef This check_type;
    typedef check_type input_type;
    typedef This prep_type;
    typedef This prep_check_type;
    typedef This bit_prep_type;
    typedef This bit_type;

    typedef RmfeMC<This> MAC_Check;
    typedef RmfeSharePrep<This> LivePrep;
    typedef RmfeInput<This> Input;
    typedef RmfeBeaver<This> Protocol;
    typedef NPartyTripleGenerator<This> TripleGenerator;
    typedef RmfeMultiplier<This> Multiplier;

    typedef void DynamicMemory;
    typedef SwitchableOutput out_type;
    typedef RmfeMC<This> MC;

    typedef BitDiagonal Rectangle;
    typedef typename T::Square Square;

    static const bool is_real = true;
    static const bool expensive_triples = true;
    static const true_type tight_packed;
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

    static ShareThread<whole_type>& get_party();

    static MAC_Check* new_mc(mac_key_type mac_key)
    {
        return new MAC_Check(mac_key);
    }

    // static void setup_rmfe(long k1, long k2) {
    //     if (Gf2RMFE::has_singleton())
    //         throw runtime_error("Can only setup RMFE once");
    //     auto rmfe = get_composite_gf2_rmfe_type2(2, 6);
    //     Gf2RMFE::set_singleton(std::move(rmfe));
    // }

    // static void teardown_rmfe() {
    //     if (!Gf2RMFE::has_singleton())
    //         return;
    //     Gf2RMFE::reset_singleton();
    // }

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
    // RmfeShare(const RmfeSecret& other);

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

    void public_input(long input);

    This get_bit(int i)
    {
        throw invalid_pack_usage();
    }

    void xor_bit(size_t i, const part_type& bit)
    {
        throw invalid_pack_usage();
    }

    void xor_(int n, const This& x, const This& y);

    void load_clear(int n, const Integer& x);

    void invert(int n, const This& x);

    This operator&(const BitVec::super& other) const;

    template<class U>
    static void trans(Processor<U>& processor, int n_outputs, const vector<int>& args);

    void extend_bit(This& res, int n_bits) const
    {
        throw runtime_error("Extending one bit is not implemented for RmfeShare.");
    }

    void mask(This& res, int n_bits) const {
        // if (n_bits != default_length)
        //     throw runtime_error("Invalid length for mask of RmfeShare: n = " + to_string(n_bits));

        // Masking in RMFE does nothing because we cannot decompose the packed and shared type.
        res = *this;
    }

    template <class U>
    void bitdec(Memory<U>& S, const vector<int>& regs) const
    {
        throw runtime_error("Bit decomposition is not implemented for RmfeShare.");
    }

    template <class U>
    void bitcom(Memory<U>& S, const vector<int>& regs)
    {
        throw runtime_error("Bit composition is not implemented for RmfeShare.");
    }

    using super::constant;
    
    static This constant(BitVec other, int my_num, mac_key_type alphai, int n_bits = -1) {
        if (n_bits < 0)
            n_bits = This::default_length;
        if (n_bits > default_length)
            throw runtime_error("Invalid length for constant of RmfeShare: n = " + to_string(n_bits));
        if (n_bits < default_length)
            other = other.mask(n_bits);
        This res = super::constant(RmfeShare::open_type(RmfeShare::clear(other)), my_num, alphai);
        return res;
    }


    template <class U>
    void finalize_input(U& inputter, int from, int n_bits)
    {
        if (n_bits != default_length)
            throw runtime_error("Invalid length for finalize_input of RmfeShare: n = " + to_string(n_bits));
        inputter.finalize(from, n_bits).mask(*this, n_bits);
    }

    /**
     * The 'and' with constant operation in RMFE protocol requires communication.
    */
    static void andm(Processor<RmfeShare>& processor, const BaseInstruction& instruction);

    RmfeShare operator<<(int i) const {
        throw runtime_error("Shifting << is not implemented for RmfeShare.");
    };

    RmfeShare operator>>(int i) const {
        throw runtime_error("Shifting >> is not implemented for RmfeShare.");
    };
};


inline const true_type RmfeShare::tight_packed;

} /* namespace GC */

#endif /* GC_RMFEHARE_H_ */
