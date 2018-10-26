/*
 * ReplicatedSecret.cpp
 *
 */

#include "ReplicatedSecret.h"
#include "ReplicatedParty.h"
#include "Thread.h"
#include "square64.h"

#include "Math/Share.h"

namespace GC
{

int ReplicatedSecret::default_length = 8 * sizeof(ReplicatedSecret::value_type);

SwitchableOutput ReplicatedSecret::out;

void ReplicatedSecret::load(int n, const Integer& x)
{
    if ((size_t)n < 8 * sizeof(x) and abs(x.get()) >= (1LL << n))
        throw out_of_range("public value too long");
    *this = x;
}

void ReplicatedSecret::bitcom(Memory<ReplicatedSecret>& S, const vector<int>& regs)
{
    *this = 0;
    for (unsigned int i = 0; i < regs.size(); i++)
        *this ^= (S[regs[i]] << i);
}

void ReplicatedSecret::bitdec(Memory<ReplicatedSecret>& S, const vector<int>& regs) const
{
    for (unsigned int i = 0; i < regs.size(); i++)
        S[regs[i]] = (*this >> i) & 1;
}

void ReplicatedSecret::load(vector<ReadAccess<ReplicatedSecret> >& accesses,
        const Memory<ReplicatedSecret>& mem)
{
    for (auto access : accesses)
        access.dest = mem[access.address];
}

void ReplicatedSecret::store(Memory<ReplicatedSecret>& mem,
        vector<WriteAccess<ReplicatedSecret> >& accesses)
{
    for (auto access : accesses)
        mem[access.address] = access.source;
}

void ReplicatedSecret::store_clear_in_dynamic(Memory<DynamicType>& mem,
        const vector<ClearWriteAccess>& accesses)
{
    for (auto access : accesses)
        mem[access.address] = access.value;
}

void ReplicatedSecret::inputb(Processor<ReplicatedSecret>& processor,
        const vector<int>& args)
{
    auto& party = ReplicatedParty::s();
    party.os.resize(2);
    for (auto& o : party.os)
        o.reset_write_head();
    processor.check_args(args, 3);
    for (size_t i = 0; i < args.size(); i += 3)
    {
        int from = args[i];
        int n_bits = args[i + 1];
        if (from == party.P->my_num())
        {
            auto& res = processor.S[args[i + 2]];
            res.prepare_input(party.os, processor.get_input(n_bits), n_bits, party.secure_prng);
        }
    }

    party.P->send_relative(party.os);
    party.P->receive_relative(party.os);

    for (size_t i = 0; i < args.size(); i += 3)
    {
        int from = args[i];
        int n_bits = args[i + 1];
        if (from != party.P->my_num())
        {
            auto& res = processor.S[args[i + 2]];
            res.finalize_input(party, party.os[party.P->get_offset(from) == 2], from, n_bits);
        }
    }
}

ReplicatedSecret ReplicatedSecret::input(int from, Processor<ReplicatedSecret>& processor, int n_bits)
{
    // BMR stuff counts from 1
    from--;
    auto& party = ReplicatedParty::s();
    ReplicatedSecret res;
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

void ReplicatedSecret::prepare_input(vector<octetStream>& os, long input, int n_bits, PRNG& secure_prng)
{
    randomize_to_sum(input, secure_prng);
    *this &= get_mask(n_bits);
    for (int i = 0; i < 2; i++)
        BitVec(get_mask(n_bits) & (*this)[i]).pack(os[i], n_bits);
}

void ReplicatedSecret::finalize_input(Thread<ReplicatedSecret>& party, octetStream& o, int from, int n_bits)
{
    int j = party.P->get_offset(from) == 2;
    (*this)[j] = BitVec::unpack_new(o, n_bits);
    (*this)[1 - j] = 0;
}

void ReplicatedSecret::and_(Processor<ReplicatedSecret>& processor,
        const vector<int>& args, bool repeat)
{
    auto& party = ReplicatedParty::s();
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

inline void ReplicatedSecret::prepare_and(vector<octetStream>& os, int n,
        const ReplicatedSecret& x, const ReplicatedSecret& y,
        Thread<ReplicatedSecret>& party, bool repeat)
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

void ReplicatedSecret::and_(int n, const ReplicatedSecret& x,
        const ReplicatedSecret& y, bool repeat)
{
    if (repeat)
        andrs(n, x, y);
    else
        throw runtime_error("call static ReplicatedSecret::ands()");
}

void ReplicatedSecret::andrs(int n, const ReplicatedSecret& x,
        const ReplicatedSecret& y)
{
    auto& party = ReplicatedParty::s();
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

inline void ReplicatedSecret::finalize_andrs(vector<octetStream>& os, int n)
{
    (*this)[1].unpack(os[1], n);
}

void ReplicatedSecret::trans(Processor<ReplicatedSecret>& processor,
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

void ReplicatedSecret::reveal(Clear& x)
{
    ReplicatedSecret share = *this;
    vector<BitVec> opened;
    auto& party = ReplicatedParty::s();
    party.MC.POpen_Begin(opened, {share}, *party.P);
    party.MC.POpen_End(opened, {share}, *party.P);
    x = IntBase(opened[0]);
}

void ReplicatedSecret::random_bit()
{
    auto& party = ReplicatedParty::s();
    *this = party.secure_prng.get_bit();
    octetStream o;
    (*this)[0].pack(o, 1);
    party.P->pass_around(o, 1);
    (*this)[1].unpack(o, 1);
}

}
