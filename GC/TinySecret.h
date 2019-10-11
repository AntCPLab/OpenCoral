/*
 * TinySecret.h
 *
 */

#ifndef GC_TINYSECRET_H_
#define GC_TINYSECRET_H_

#include "Secret.h"
#include "TinyShare.h"
#include "ShareParty.h"
#include "OT/Rectangle.h"
#include "OT/BitDiagonal.h"

template<class T> class NPartyTripleGenerator;
template<class T> class OTTripleGenerator;
template<class T> class TinyMultiplier;

namespace GC
{

template<class T> class TinyPrep;
template<class T> class TinyMC;

template<int S>
class TinySecret : public Secret<TinyShare<S>>
{
    typedef TinySecret This;

public:
    typedef TinyShare<S> part_type;
    typedef Secret<part_type> super;

    typedef typename part_type::mac_key_type mac_key_type;

    typedef BitVec open_type;
    typedef BitVec clear;

    typedef TinyMC<This> MC;
    typedef MC MAC_Check;
    typedef Beaver<This> Protocol;
    typedef ::Input<This> Input;
    typedef TinyPrep<This> LivePrep;
    typedef Memory<This> DynamicMemory;

    typedef OTTripleGenerator<This> TripleGenerator;
    typedef TinyMultiplier<This> Multiplier;
    typedef typename part_type::sacri_type sacri_type;
    typedef typename part_type::mac_type mac_type;
    typedef BitDiagonal Rectangle;

    static const bool dishonest_majority = true;
    static const bool needs_ot = true;

    static const int default_length = 64;

    static string type_short()
    {
        return "T";
    }

    static DataFieldType field_type()
    {
        return BitVec::field_type();
    }

    static int size()
    {
        return part_type::size() * default_length;
    }

    static MC* new_mc(Machine<This>& machine)
    {
        (void) machine;
        return new MC(ShareParty<This>::s().mac_key);
    }

    static void store_clear_in_dynamic(Memory<This>& mem,
            const vector<ClearWriteAccess>& accesses)
    {
        auto& party = ShareThread<This>::s();
        for (auto access : accesses)
            mem[access.address] = constant(access.value, party.P->my_num(),
                    {});
    }

    static This constant(BitVec other, int my_num, mac_key_type alphai)
    {
        This res;
        res.resize_regs(other.length());
        for (int i = 0; i < other.length(); i++)
            res.get_reg(i) = part_type::constant(other.get_bit(i), my_num, alphai);
        return res;
    }

    TinySecret()
    {
    }
    TinySecret(const super& other) :
            super(other)
    {
    }

    void assign(const char* buffer)
    {
        this->resize_regs(default_length);
        for (int i = 0; i < default_length; i++)
            this->get_reg(i).assign(buffer + i * part_type::size());
    }

    This operator-(const This& other) const
    {
        return *this + other;
    }

    This operator*(const BitVec& other) const
    {
        This res = *this;
        for (int i = 0; i < super::size(); i++)
            if (not other.get_bit(i))
                res.get_reg(i) = {};
        return res;
    }

    This extend_bit() const
    {
        This res;
        res.get_regs().resize(BitVec::N_BITS, this->get_reg(0));
        return res;
    }

    This mask(int n_bits) const
    {
        This res = *this;
        res.get_regs().resize(n_bits);
        return res;
    }

    void reveal(size_t n_bits, Clear& x)
    {
        auto& to_open = *this;
        to_open.resize_regs(n_bits);
        auto& party = ShareThread<This>::s();
        x = party.MC->POpen(to_open, *party.P);
    }

    void output(ostream& s, bool human = true) const
    {
        assert(this->get_regs().size() == default_length);
        for (auto& reg : this->get_regs())
            reg.output(s, human);
    }
};

template<int S>
inline TinySecret<S> operator*(const BitVec& clear, const TinySecret<S>& share)
{
    return share * clear;
}

} /* namespace GC */

#endif /* GC_TINYSECRET_H_ */
