/*
 * ReplicatedSecret.cpp
 *
 */

#include "ShareSecret.h"

#include "MaliciousRepSecret.h"
#include "Protocols/MaliciousRepMC.h"
#include "ShareThread.h"
#include "Thread.h"
#include "square64.h"

#include "Protocols/Share.h"

#include "Protocols/ReplicatedMC.hpp"
#include "Protocols/Beaver.hpp"
#include "ShareParty.h"
#include "ShareThread.hpp"

namespace GC
{

template<class U>
int ReplicatedSecret<U>::default_length = 8 * sizeof(typename ReplicatedSecret<U>::value_type);

template<class U>
SwitchableOutput ShareSecret<U>::out;

template<class U>
void ShareSecret<U>::check_length(int n, const Integer& x)
{
    if ((size_t)n < 8 * sizeof(x) and abs(x.get()) >= (1LL << n))
        throw out_of_range("public value too long");
}

template<class U>
void ReplicatedSecret<U>::load_clear(int n, const Integer& x)
{
    this->check_length(n, x);
    *this = x;
}

template<class U>
void ReplicatedSecret<U>::bitcom(Memory<U>& S, const vector<int>& regs)
{
    *this = 0;
    for (unsigned int i = 0; i < regs.size(); i++)
        *this ^= (S[regs[i]] << i);
}

template<class U>
void ReplicatedSecret<U>::bitdec(Memory<U>& S, const vector<int>& regs) const
{
    for (unsigned int i = 0; i < regs.size(); i++)
        S[regs[i]] = (*this >> i) & 1;
}

template<class U>
void ShareSecret<U>::load(vector<ReadAccess<U> >& accesses,
        const Memory<U>& mem)
{
    for (auto access : accesses)
        access.dest = mem[access.address];
}

template<class U>
void ShareSecret<U>::store(Memory<U>& mem,
        vector<WriteAccess<U> >& accesses)
{
    for (auto access : accesses)
        mem[access.address] = access.source;
}

template<class U>
void ShareSecret<U>::store_clear_in_dynamic(Memory<U>& mem,
        const vector<ClearWriteAccess>& accesses)
{
    for (auto access : accesses)
        mem[access.address] = access.value;
}

template<class U>
void ShareSecret<U>::inputb(Processor<U>& processor,
        const vector<int>& args)
{
    auto& party = ShareThread<U>::s();
    typename U::Input input(*party.MC, party.DataF, *party.P);
    input.reset_all(*party.P);

    InputArgList a(args);
    bool interactive = party.n_interactive_inputs_from_me(a) > 0;

    for (auto x : a)
    {
        if (x.from == party.P->my_num())
        {
            input.add_mine(processor.get_input(x.params, interactive), x.n_bits);
        }
        else
            input.add_other(x.from);
    }

    if (interactive)
        cout << "Thank you" << endl;

    input.exchange();

    for (auto x : a)
    {
        int from = x.from;
        int n_bits = x.n_bits;
        auto& res = processor.S[x.dest];
        res = input.finalize(from, n_bits).mask(n_bits);
    }
}

template<class U>
BitVec ReplicatedSecret<U>::local_mul(const ReplicatedSecret& other) const
{
    return (*this)[0] * other.sum() + (*this)[1] * other[0];
}

template<class U>
void ShareSecret<U>::and_(
        Processor<U>& processor, const vector<int>& args,
        bool repeat)
{
    ShareThread<U>::s().and_(processor, args, repeat);
}

template<class U>
void ReplicatedSecret<U>::trans(Processor<U>& processor,
        int n_outputs, const vector<int>& args)
{
    assert(length == 2);
    for (int k = 0; k < 2; k++)
    {
        square64 square;
        for (size_t i = n_outputs; i < args.size(); i++)
            square.rows[i - n_outputs] = processor.S[args[i]][k].get();
        square.transpose(args.size() - n_outputs, n_outputs);
        for (int i = 0; i < n_outputs; i++)
            processor.S[args[i]][k] = square.rows[i];
    }
}

template<class U>
void ReplicatedSecret<U>::reveal(size_t n_bits, Clear& x)
{
    (void) n_bits;
    auto& share = *this;
    vector<BitVec> opened;
    auto& party = ShareThread<U>::s();
    party.MC->POpen(opened, {share}, *party.P);
    x = IntBase(opened[0]);
}

template<class U>
void ShareSecret<U>::random_bit()
{
    U res;
    ShareThread<U>::s().DataF.get_one(DATA_BIT, res);
    *this = res;
}

}
