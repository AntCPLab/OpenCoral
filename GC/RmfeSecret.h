/*
 * RmfeSecret.h
 *
 */

#ifndef GC_RMFESECRET_H_
#define GC_RMFESECRET_H_

#include "TinySecret.h"
#include "RmfeShare.h"
#include "VectorPrep.h"
#include "RmfeMultiInput.h"
#include "RmfeVectorMC.h" 
#include "RmfeVectorProtocol.h"
#include "RmfeMultiplier.h"


namespace GC
{

class RmfeSecret : public VectorSecret<RmfeShare>
{
    typedef VectorSecret<RmfeShare> super;
    typedef RmfeSecret This;

public:
    typedef RmfeShare::T T;
    typedef RmfeVectorMC<This> MC;
    typedef MC MAC_Check;
    typedef RmfeVectorProtocol<This> Protocol;
    typedef RmfeMultiInput<This> Input;
    typedef VectorPrep<This> LivePrep;
    typedef Memory<This> DynamicMemory;

    typedef NPartyTripleGenerator<This> TripleGenerator;
    typedef NPartyTripleGenerator<This> InputGenerator;
    typedef RmfeMultiplier<This> Multiplier;
    typedef typename T::Square Square;

    typedef typename super::part_type check_type;
    typedef Share<T> input_check_type;
    typedef check_type input_type;

    static string type_short()
    {
        return "TT";
    }

    static MC* new_mc(typename super::mac_key_type mac_key)
    {
        return new MC(mac_key);
    }

    template<class U>
    static void generate_mac_key(typename super::mac_key_type& dest, const U&)
    {
        SeededPRNG G;
        dest.randomize(G);
    }

    static void store_clear_in_dynamic(Memory<This>& mem,
            const vector<ClearWriteAccess>& accesses)
    {
        auto& party = ShareThread<This>::s();
        for (auto access : accesses)
            mem[access.address] = super::constant(access.value,
                    party.P->my_num(), {});
    }


    RmfeSecret()
    {
    }
    RmfeSecret(const super& other) :
            super(other)
    {
    }
    RmfeSecret(const typename super::super& other) :
            super(other)
    {
    }
    RmfeSecret(const typename super::part_type& other)
    {
        this->get_regs().push_back(other);
    }

    void reveal(size_t n_bits, Clear& x)
    {
        auto& to_open = *this;
        to_open.resize_regs(n_bits);
        auto& party = ShareThread<This>::s();
        x = party.MC->POpen(to_open, *party.P);
    }

    RmfeShare get_part(int i) const
    {
        return this->get_reg(i);
    }
};

RmfeShare::RmfeShare(const RmfeSecret& other)
{
    assert(other.get_regs().size() > 0);
    *this = other.get_reg(0);
}

} /* namespace GC */

#endif /* GC_RMFESECRET_H_ */
