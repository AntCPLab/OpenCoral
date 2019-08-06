/*
 * ReplicatedSecret.cpp
 *
 */

#include "ReplicatedSecret.h"
#include "ReplicatedParty.h"
#include "MaliciousRepSecret.h"
#include "Protocols/MaliciousRepMC.h"
#include "MaliciousRepThread.h"
#include "Thread.h"
#include "square64.h"

#include "Protocols/Share.h"

#include "Protocols/ReplicatedMC.hpp"
#include "Protocols/Replicated.hpp"

namespace GC
{

template<class U>
int ReplicatedSecret<U>::default_length = 8 * sizeof(ReplicatedSecret<U>::value_type);

template<class U>
SwitchableOutput ReplicatedSecret<U>::out;

template<class U>
void ReplicatedSecret<U>::load(int n, const Integer& x)
{
    if ((size_t)n < 8 * sizeof(x) and abs(x.get()) >= (1LL << n))
        throw out_of_range("public value too long");
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
void ReplicatedSecret<U>::load(vector<ReadAccess<U> >& accesses,
        const Memory<U>& mem)
{
    for (auto access : accesses)
        access.dest = mem[access.address];
}

template<class U>
void ReplicatedSecret<U>::store(Memory<U>& mem,
        vector<WriteAccess<U> >& accesses)
{
    for (auto access : accesses)
        mem[access.address] = access.source;
}

template<class U>
void ReplicatedSecret<U>::store_clear_in_dynamic(Memory<U>& mem,
        const vector<ClearWriteAccess>& accesses)
{
    for (auto access : accesses)
        mem[access.address] = access.value;
}

template<class U>
void ReplicatedSecret<U>::inputb(Processor<U>& processor,
        const vector<int>& args)
{
    auto& party = ReplicatedParty<U>::s();
    party.os.resize(2);
    for (auto& o : party.os)
        o.reset_write_head();

    InputArgList a(args);
    bool interactive = party.n_interactive_inputs_from_me(a) > 0;

    for (auto x : a)
    {
        if (x.from == party.P->my_num())
        {
            auto& res = processor.S[x.dest];
            res.prepare_input(party.os, processor.get_input(x.params, interactive), x.n_bits, party.secure_prng);
        }
    }

    if (interactive)
        cout << "Thank you" << endl;

    for (int i = 0; i < 2; i++)
        party.P->pass_around(party.os[i], i + 1);

    for (auto x : a)
    {
        int from = x.from;
        int n_bits = x.n_bits;
        if (from != party.P->my_num())
        {
            auto& res = processor.S[x.dest];
            res.finalize_input(party, party.os[party.P->get_offset(from) == 1], from, n_bits);
        }
    }
}

template<class U>
U ReplicatedSecret<U>::input(Processor<U>& processor, const InputArgs& args)
{
    int from = args.from;
    int n_bits = args.n_bits;
    auto& party = ReplicatedParty<U>::s();
    U res;
    party.os.resize(2);
    for (auto& o : party.os)
        o.reset_write_head();
    if (from == party.P->my_num())
    {
        res.prepare_input(party.os, processor.get_input(args.params), n_bits, party.secure_prng);
        party.P->send_relative(party.os);
    }
    else
    {
        party.P->receive_player(from, party.os[0], true);
        res.finalize_input(party, party.os[0], from, n_bits);
    }
    return res;
}

template<class U>
void ReplicatedSecret<U>::prepare_input(vector<octetStream>& os, long input, int n_bits, PRNG& secure_prng)
{
    randomize_to_sum(input, secure_prng);
    *this &= get_mask(n_bits);
    for (int i = 0; i < 2; i++)
        BitVec(get_mask(n_bits) & (*this)[i]).pack(os[i], n_bits);
}

template<class U>
void ReplicatedSecret<U>::finalize_input(Thread<U>& party, octetStream& o, int from, int n_bits)
{
    int j = party.P->get_offset(from) == 2;
    (*this)[j] = BitVec::unpack_new(o, n_bits);
    (*this)[1 - j] = 0;
}

template<class U>
BitVec ReplicatedSecret<U>::local_mul(const ReplicatedSecret& other) const
{
    return (*this)[0] * other.sum() + (*this)[1] * other[0];
}

template<class U>
void ReplicatedSecret<U>::and_(int n,
        const ReplicatedSecret<U>& x,
        const ReplicatedSecret<U>& y, bool repeat)
{
    (void)n, (void)x, (void)y, (void)repeat;
    throw runtime_error("use static method");
}

template<>
void ReplicatedSecret<SemiHonestRepSecret>::and_(Processor<SemiHonestRepSecret>& processor,
        const vector<int>& args, bool repeat)
{
    auto& party = Thread<SemiHonestRepSecret>::s();
    assert(party.P->num_players() == 3);
    processor.check_args(args, 4);
    assert(party.protocol != 0);
    auto& protocol = *party.protocol;
    protocol.init_mul();
    for (size_t i = 0; i < args.size(); i += 4)
    {
        int n_bits = args[i];
        int left = args[i + 2];
        int right = args[i + 3];
        MaliciousRepSecret y_ext;
        if (repeat)
            y_ext = processor.S[right].extend_bit();
        else
            y_ext = processor.S[right];
        protocol.prepare_mul(processor.S[left].mask(n_bits), y_ext.mask(n_bits), n_bits);
    }
    protocol.exchange();
    for (size_t i = 0; i < args.size(); i += 4)
        processor.S[args[i + 1]] = protocol.finalize_mul(args[i]);
}

template<>
void ReplicatedSecret<MaliciousRepSecret>::and_(
        Processor<MaliciousRepSecret>& processor, const vector<int>& args,
        bool repeat)
{
    MaliciousRepThread::s().and_(processor, args, repeat);
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
    ReplicatedSecret share = *this;
    vector<BitVec> opened;
    auto& party = ReplicatedParty<U>::s();
    party.MC->POpen_Begin(opened, {share}, *party.P);
    party.MC->POpen_End(opened, {share}, *party.P);
    x = IntBase(opened[0]);
}

template<>
void ReplicatedSecret<SemiHonestRepSecret>::random_bit()
{
    auto& party = ReplicatedParty<SemiHonestRepSecret>::s();
    *this = party.secure_prng.get_bit();
    octetStream o;
    (*this)[0].pack(o, 1);
    party.P->pass_around(o, 1);
    (*this)[1].unpack(o, 1);
}

template<>
void ReplicatedSecret<MaliciousRepSecret>::random_bit()
{
    MaliciousRepSecret res;
    MaliciousRepThread::s().DataF.get_one(DATA_BIT, res);
    *this = res;
}

template class ReplicatedSecret<SemiHonestRepSecret>;
template class ReplicatedSecret<MaliciousRepSecret>;

}
