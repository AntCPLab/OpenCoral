/*
 * Replicated.cpp
 *
 */

#include "Replicated.h"
#include "Processor.h"
#include "Math/FixedVec.h"
#include "Math/Integer.h"
#include "Math/MaliciousRep3Share.h"
#include "Tools/benchmarking.h"
#include "GC/ReplicatedSecret.h"

template<class T>
Replicated<T>::Replicated(Player& P) : ReplicatedBase(P), counter(0)
{
    assert(T::length == 2);
}

ReplicatedBase::ReplicatedBase(Player& P) : P(P)
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

template<class T>
inline Replicated<T>::~Replicated()
{
    cerr << "Number of multiplications: " << counter << endl;
}

template<class T>
void Replicated<T>::muls(const vector<int>& reg,
        SubProcessor<T>& proc, ReplicatedMC<T>& MC, int size)
{
    (void)MC;
    assert(T::length == 2);
    assert(reg.size() % 3 == 0);
    int n = reg.size() / 3;

    init_mul();
    for (int i = 0; i < n; i++)
        for (int j = 0; j < size; j++)
        {
            auto& x = proc.S[reg[3 * i + 1] + j];
            auto& y = proc.S[reg[3 * i + 2] + j];
            prepare_mul(x, y);
        }
    exchange();
    for (int i = 0; i < n; i++)
        for (int j = 0; j < size; j++)
        {
            proc.S[reg[3 * i] + j] = finalize_mul();
        }

    counter += n * size;
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
typename T::clear Replicated<T>::prepare_mul(const T& x,
        const T& y)
{
    typename T::value_type add_share = x[0] * y.sum() + x[1] * y[0];
    typename T::value_type tmp[2];
    for (int i = 0; i < 2; i++)
        tmp[i].randomize(shared_prngs[i]);
    add_share += tmp[0] - tmp[1];
    add_share.pack(os[0]);
    add_shares.push_back(add_share);
    return add_share;
}

template<class T>
void Replicated<T>::exchange()
{
    P.send_relative(1, os[0]);
    P.receive_relative(- 1, os[0]);
}

template<class T>
T Replicated<T>::finalize_mul()
{
    T result;
    result[0] = add_shares.front();
    add_shares.pop_front();
    result[1].unpack(os[0]);
    return result;
}

template<class T>
T Replicated<T>::get_random()
{
    T res;
    for (int i = 0; i < 2; i++)
        res[i].randomize(shared_prngs[i]);
    return res;
}

template class Replicated<Rep3Share<Integer>>;
template class Replicated<Rep3Share<gfp>>;
template class Replicated<Rep3Share<gf2n>>;
template class Replicated<MaliciousRep3Share<gfp>>;
template class Replicated<MaliciousRep3Share<gf2n>>;
