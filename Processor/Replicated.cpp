/*
 * Replicated.cpp
 *
 */

#include "Replicated.h"
#include "Processor.h"
#include "Math/FixedVec.h"
#include "Math/Integer.h"
#include "Tools/benchmarking.h"
#include "GC/ReplicatedSecret.h"

template<class T>
Replicated<T>::Replicated(Player& P) : ReplicatedBase(P), counter(0)
{
}

ReplicatedBase::ReplicatedBase(Player& P)
{
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
    cout << "Number of multiplications: " << counter << endl;
}

template<class T>
void Replicated<T>::muls(const vector<int>& reg,
        SubProcessor<T>& proc, ReplicatedMC<T>& MC, int size)
{
    (void)MC;
    assert(T::length == 2);
    assert(reg.size() % 3 == 0);
    int n = reg.size() / 3;

    os.resize(2);
    for (auto& o : os)
        o.reset_write_head();
    results.resize(n * size);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < size; j++)
        {
            auto& x = proc.S[reg[3 * i + 1] + j];
            auto& y = proc.S[reg[3 * i + 2] + j];
            typename T::value_type add_share = x[0] * y.sum() + x[1] * y[0];
            typename T::value_type tmp[2];
            for (int i = 0; i < 2; i++)
                tmp[i].randomize(shared_prngs[i]);
            add_share += tmp[0] - tmp[1];
            add_share.pack(os[0]);
            auto& result = results[i * size + j];
            result[0] = add_share;
        }
    proc.P.send_relative(1, os[0]);
    proc.P.receive_relative(- 1, os[0]);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < size; j++)
        {
            auto& result = results[i * size + j];
            result[1].unpack(os[0]);
            proc.S[reg[3 * i] + j] = result;
        }

    counter += n * size;
}

template<>
void Replicated<Rep3Share>::reqbl(int n)
{
    if ((int)n < 0 && Integer::size() * 8 != -(int)n)
    {
        throw Processor_Error(
                "Program compiled for rings of length " + to_string(-(int)n)
                + " but VM supports only "
                + to_string(Integer::size() * 8));
    }
    else if ((int)n > 0)
    {
        throw Processor_Error("Program compiled for fields not rings");
    }
}

template<class T>
inline void Replicated<T>::input(SubProcessor<T>& Proc, int n, int* r)
{
    (void)Proc;
    (void)n;
    (void)r;
    throw not_implemented();
}

template class Replicated<Rep3Share>;
