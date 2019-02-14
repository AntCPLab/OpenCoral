/*
 * Replicated.cpp
 *
 */

#include "Replicated.h"
#include "Processor.h"
#include "Math/FixedVec.h"
#include "Math/Integer.h"
#include "Math/MaliciousRep3Share.h"
#include "Math/ShamirShare.h"
#include "Tools/benchmarking.h"
#include "GC/ReplicatedSecret.h"

template<class T>
ProtocolBase<T>::ProtocolBase() : counter(0)
{
}

template<class T>
Replicated<T>::Replicated(Player& P) : ReplicatedBase(P)
{
    assert(T::length == 2);
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

template<class T>
ProtocolBase<T>::~ProtocolBase()
{
    if (counter)
        cerr << "Number of multiplications: " << counter << endl;
}

template<class T>
void ProtocolBase<T>::muls(const vector<int>& reg,
        SubProcessor<T>& proc, MAC_Check_Base<T>& MC, int size)
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
void ProtocolBase<T>::dotprods(const vector<int>& reg,
        SubProcessor<T>& proc)
{
    proc.dotprods(reg);
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
void Replicated<T>::init_mul()
{
    os.resize(2);
    for (auto& o : os)
        o.reset_write_head();
    add_shares.clear();
}

template<class T>
inline typename T::clear Replicated<T>::prepare_mul(const T& x,
        const T& y)
{
    typename T::value_type add_share = x.local_mul(y);
    prepare_reshare(add_share);
    return add_share;
}

template<class T>
inline void Replicated<T>::prepare_reshare(const typename T::clear& share)
{
    auto add_share = share;
    typename T::value_type tmp[2];
    for (int i = 0; i < 2; i++)
        tmp[i].randomize(shared_prngs[i]);
    add_share += tmp[0] - tmp[1];
    add_share.pack(os[0]);
    add_shares.push_back(add_share);
}

template<class T>
void Replicated<T>::exchange()
{
    P.pass_around(os[0], 1);
}

template<class T>
inline T Replicated<T>::finalize_mul()
{
    T result;
    result[0] = add_shares.front();
    add_shares.pop_front();
    result[1].unpack(os[0]);
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
