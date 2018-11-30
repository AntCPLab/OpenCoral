/*
 * ReplicatedSecret.cpp
 *
 */

#include "ReplicatedSecret.h"
#include "ReplicatedParty.h"
#include "MaliciousRepSecret.h"
#include "Auth/MaliciousRepMC.h"
#include "MaliciousRepThread.h"
#include "Thread.h"
#include "square64.h"

#include "Math/Share.h"

#include "Auth/ReplicatedMC.hpp"

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
    processor.check_args(args, 3);

    InputArgList a(args);
    bool interactive = party.n_interactive_inputs_from_me(a) > 0;

    for (size_t i = 0; i < args.size(); i += 3)
    {
        int from = args[i];
        int n_bits = args[i + 1];
        if (from == party.P->my_num())
        {
            auto& res = processor.S[args[i + 2]];
            res.prepare_input(party.os, processor.get_input(n_bits, interactive), n_bits, party.secure_prng);
        }
    }

    if (interactive)
        cout << "Thank you" << endl;

    for (int i = 0; i < 2; i++)
        party.P->pass_around(party.os[i], i + 1);

    for (size_t i = 0; i < args.size(); i += 3)
    {
        int from = args[i];
        int n_bits = args[i + 1];
        if (from != party.P->my_num())
        {
            auto& res = processor.S[args[i + 2]];
            res.finalize_input(party, party.os[party.P->get_offset(from) == 1], from, n_bits);
        }
    }
}

template<class U>
U ReplicatedSecret<U>::input(int from, Processor<U>& processor, int n_bits)
{
    // BMR stuff counts from 1
    from--;
    auto& party = ReplicatedParty<U>::s();
    U res;
    party.os.resize(2);
    for (auto& o : party.os)
        o.reset_write_head();
    if (from == party.P->my_num())
    {
        res.prepare_input(party.os, processor.get_input(n_bits), n_bits, party.secure_prng);
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

template<>
inline void ReplicatedSecret<SemiHonestRepSecret>::prepare_and(vector<octetStream>& os, int n,
        const ReplicatedSecret<SemiHonestRepSecret>& x, const ReplicatedSecret<SemiHonestRepSecret>& y,
        Thread<SemiHonestRepSecret>& party, bool repeat)
{
    ReplicatedSecret y_ext;
    if (repeat)
        y_ext = y.extend_bit();
    else
        y_ext = y;
    auto add_share = x[0] * y_ext.sum() + x[1] * y_ext[0];
    BitVec tmp[2];
    for (int i = 0; i < 2; i++)
        tmp[i].randomize(party.protocol->shared_prngs[i]);
    add_share += tmp[0] - tmp[1];
    (*this)[0] = add_share;
    BitVec mask = get_mask(n);
    *this &= mask;
    BitVec(mask & (*this)[0]).pack(os[0], n);
}

template<>
inline void ReplicatedSecret<SemiHonestRepSecret>::finalize_andrs(
        vector<octetStream>& os, int n)
{
    (*this)[1].unpack(os[1], n);
}

template<>
void ReplicatedSecret<SemiHonestRepSecret>::andrs(int n,
        const ReplicatedSecret<SemiHonestRepSecret>& x,
        const ReplicatedSecret<SemiHonestRepSecret>& y)
{
    auto& party = Thread<SemiHonestRepSecret>::s();
    assert(party.P->num_players() == 3);
    vector<octetStream>& os = party.os;
    os.resize(2);
    for (auto& o : os)
        o.reset_write_head();
    prepare_and(os, n, x, y, party, true);
    party.P->send_relative(os);
    party.P->receive_relative(os);
    finalize_andrs(os, n);
}

template<>
void ReplicatedSecret<SemiHonestRepSecret>::and_(int n,
        const ReplicatedSecret<SemiHonestRepSecret>& x,
        const ReplicatedSecret<SemiHonestRepSecret>& y, bool repeat)
{
    if (repeat)
        andrs(n, x, y);
    else
        throw runtime_error("call static ReplicatedSecret::ands()");
}

template<>
void ReplicatedSecret<MaliciousRepSecret>::and_(int n,
        const ReplicatedSecret<MaliciousRepSecret>& x,
        const ReplicatedSecret<MaliciousRepSecret>& y, bool repeat)
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
    vector<octetStream>& os = party.os;
    os.resize(2);
    for (auto& o : os)
        o.reset_write_head();
    processor.check_args(args, 4);
    for (size_t i = 0; i < args.size(); i += 4)
        processor.S[args[i + 1]].prepare_and(os, args[i],
                processor.S[args[i + 2]], processor.S[args[i + 3]],
                party, repeat);
    party.P->send_relative(os);
    party.P->receive_relative(os);
    for (size_t i = 0; i < args.size(); i += 4)
        processor.S[args[i + 1]].finalize_andrs(os, args[i]);
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
void ReplicatedSecret<U>::reveal(Clear& x)
{
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
