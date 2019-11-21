/*
 * Replicated.cpp
 *
 */

#ifndef PROTOCOLS_REPLICATED_HPP_
#define PROTOCOLS_REPLICATED_HPP_

#include "Replicated.h"
#include "Processor/Processor.h"
#include "Tools/benchmarking.h"

#include "SemiShare.h"
#include "SemiMC.h"
#include "ReplicatedInput.h"
#include "Rep3Share.h"

#include "SemiMC.hpp"
#include "Math/Z2k.hpp"

template<class T>
ProtocolBase<T>::ProtocolBase() : counter(0)
{
}

template<class T>
Replicated<T>::Replicated(Player& P) : ReplicatedBase(P)
{
    assert(T::length == 2);
}

template<class T>
Replicated<T>::Replicated(const ReplicatedBase& other) :
        ReplicatedBase(other)
{
}

inline ReplicatedBase::ReplicatedBase(Player& P) : P(P)
{
    assert(P.num_players() == 3);
	if (not P.is_encrypted())
		insecure("unencrypted communication");

	shared_prngs[0].ReSeed();
	octetStream os;
	os.append(shared_prngs[0].get_seed(), SEED_SIZE);
	P.send_relative(1, os);
	P.receive_relative(-1, os);
	shared_prngs[1].SetSeed(os.get_data());
}

inline ReplicatedBase::ReplicatedBase(Player& P, array<PRNG, 2>& prngs) :
        P(P)
{
    for (int i = 0; i < 2; i++)
        shared_prngs[i].SetSeed(prngs[i]);
}

inline ReplicatedBase ReplicatedBase::branch()
{
    return {P, shared_prngs};
}

template<class T>
ProtocolBase<T>::~ProtocolBase()
{
#ifdef VERBOSE
    if (counter)
        cerr << "Number of multiplications: " << counter << endl;
#endif
}

template<class T>
void ProtocolBase<T>::muls(const vector<int>& reg,
        SubProcessor<T>& proc, typename T::MAC_Check& MC, int size)
{
    (void)MC;
    proc.muls(reg, size);
}

template<class T>
void ProtocolBase<T>::mulrs(const vector<int>& reg,
        SubProcessor<T>& proc)
{
    proc.mulrs(reg);
}

template<class T>
T ProtocolBase<T>::finalize_dotprod(int length)
{
    counter += length;
    T res;
    for (int i = 0; i < length; i++)
        res += finalize_mul();
    return res;
}

template<class T>
void Replicated<T>::init_mul(SubProcessor<T>* proc)
{
    (void) proc;
    init_mul();
}

template<class T>
void Replicated<T>::init_mul(Preprocessing<T>& prep, typename T::MAC_Check& MC)
{
    (void) prep, (void) MC;
    init_mul();
}

template<class T>
void Replicated<T>::init_mul()
{
    os.resize(2);
    for (auto& o : os)
        o.reset_write_head();
    add_shares.clear();
}

template<class T>
inline typename T::clear Replicated<T>::prepare_mul(const T& x,
        const T& y, int n)
{
    typename T::value_type add_share = x.local_mul(y);
    prepare_reshare(add_share, n);
    return add_share;
}

template<class T>
inline void Replicated<T>::prepare_reshare(const typename T::clear& share,
        int n)
{
    auto add_share = share;
    typename T::value_type tmp[2];
    for (int i = 0; i < 2; i++)
        tmp[i].randomize(shared_prngs[i], n);
    add_share += tmp[0] - tmp[1];
    add_share.pack(os[0], n);
    add_shares.push_back(add_share);
}

template<class T>
void Replicated<T>::exchange()
{
    P.pass_around(os[0], os[1], 1);
}

template<class T>
void Replicated<T>::start_exchange()
{
    P.send_relative(1, os[0]);
}

template<class T>
void Replicated<T>::stop_exchange()
{
    P.receive_relative(-1, os[1]);
}

template<class T>
inline T Replicated<T>::finalize_mul(int n)
{
    T result;
    result[0] = add_shares.front();
    add_shares.pop_front();
    result[1].unpack(os[1], n);
    return result;
}

template<class T>
inline void Replicated<T>::init_dotprod(SubProcessor<T>* proc)
{
    init_mul(proc);
    dotprod_share.assign_zero();
}

template<class T>
inline void Replicated<T>::prepare_dotprod(const T& x, const T& y)
{
    dotprod_share += x.local_mul(y);
}

template<class T>
inline void Replicated<T>::next_dotprod()
{
    prepare_reshare(dotprod_share);
    dotprod_share.assign_zero();
}

template<class T>
inline T Replicated<T>::finalize_dotprod(int length)
{
    (void) length;
    this->counter++;
    return finalize_mul();
}

template<class T>
T Replicated<T>::get_random()
{
    T res;
    for (int i = 0; i < 2; i++)
        res[i].randomize(shared_prngs[i]);
    return res;
}

template<int K>
void trunc_pr(const vector<int>& regs, int size,
        SubProcessor<Rep3Share<SignedZ2<K>>>& proc)
{
    assert(regs.size() % 4 == 0);
    assert(proc.P.num_players() == 3);
    typedef SignedZ2<K> value_type;
    typedef Rep3Share<value_type> T;
    bool generate = proc.P.my_num() == 2;
    if (generate)
    {
        octetStream os[2];
        for (size_t i = 0; i < regs.size(); i += 4)
        {
            int k = regs[i + 2];
            int m = regs[i + 3];
            int n_shift = value_type::N_BITS - 1 - k;
            assert(m < k);
            assert(0 < k);
            assert(m < value_type::N_BITS);
            for (int l = 0; l < size; l++)
            {
                auto& res = proc.get_S_ref(regs[i] + l);
                auto& G = proc.Proc.secure_prng;
                auto mask = G.template get<value_type>();
                auto unmask = (mask << (n_shift + 1)) >> (n_shift + m + 1);
                T shares[4];
                shares[0].randomize_to_sum(mask, G);
                shares[1].randomize_to_sum(unmask, G);
                shares[2].randomize_to_sum(
                        (mask << (n_shift)) >> (value_type::N_BITS - 1), G);
                res.randomize(G);
                shares[3] = res;
                for (int i = 0; i < 2; i++)
                {
                    for (int j = 0; j < 4; j++)
                        shares[j][i].pack(os[i]);
                }
            }
        }
        for (int i = 0; i < 2; i++)
            proc.P.send_to(i, os[i], true);
    }
    else
    {
        octetStream os;
        proc.P.receive_player(2, os, true);
        OffsetPlayer player(proc.P, 1 - 2 * proc.P.my_num());
        typedef SemiShare<value_type> semi_type;
        vector<SemiShare<value_type>> to_open;
        PointerVector<SemiShare<value_type>> mask_shares[3];
        for (size_t i = 0; i < regs.size(); i += 4)
            for (int l = 0; l < size; l++)
            {
                SemiShare<value_type> share;
                auto& x = proc.get_S_ref(regs[i + 1] + l);
                if (proc.P.my_num() == 0)
                    share = x.sum();
                else
                    share = x[0];
                for (auto& mask_share : mask_shares)
                    mask_share.push_back(os.get<semi_type>());
                to_open.push_back(share + mask_shares[0].next());
                auto& res = proc.get_S_ref(regs[i] + l);
                auto& a = res[1 - proc.P.my_num()];
                a.unpack(os);
            }
        PointerVector<value_type> opened;
        DirectSemiMC<SemiShare<value_type>> MC;
        MC.POpen_(opened, to_open, player);
        os.reset_write_head();
        for (size_t i = 0; i < regs.size(); i += 4)
        {
            int k = regs[i + 2];
            int m = regs[i + 3];
            int n_shift = value_type::N_BITS - 1 - k;
            assert(m < k);
            assert(0 < k);
            assert(m < value_type::N_BITS);
            for (int l = 0; l < size; l++)
            {
                auto& res = proc.get_S_ref(regs[i] + l);
                auto masked = opened.next() << n_shift;
                auto shifted = (masked << 1) >> (n_shift + m + 1);
                auto diff = SemiShare<value_type>::constant(shifted,
                        player.my_num()) - mask_shares[1].next();
                auto msb = masked >> (value_type::N_BITS - 1);
                auto bit_mask = mask_shares[2].next();
                auto overflow = (bit_mask
                        + SemiShare<value_type>::constant(msb, player.my_num())
                        - bit_mask * msb * 2);
                auto res_share = diff + (overflow << (k - m));
                auto& a = res[1 - proc.P.my_num()];
                auto& b = res[proc.P.my_num()];
                b = res_share - a;
                b.pack(os);
            }
        }
        player.exchange(os);
        for (size_t i = 0; i < regs.size(); i += 4)
            for (int l = 0; l < size; l++)
                proc.get_S_ref(regs[i] + l)[proc.P.my_num()] +=
                        os.get<value_type>();
    }
}

template<class T>
void trunc_pr(const vector<int>& regs, int size, SubProcessor<T>& proc)
{
    (void) regs, (void) size, (void) proc;
    throw not_implemented();
}

template<class T>
void Replicated<T>::trunc_pr(const vector<int>& regs, int size,
        SubProcessor<T>& proc)
{
    ::trunc_pr(regs, size, proc);
}

#endif
