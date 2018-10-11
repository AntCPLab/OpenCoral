/*
 * Replicated.cpp
 *
 */

#include "Replicated.h"
#include "Processor.h"
#include "Math/FixedVec.h"
#include "Math/Integer.h"
#include "Tools/benchmarking.h"

template<class T>
Replicated<T>::Replicated() : counter(0)
{
	insecure("unencrypted communication");
}

template<class T>
inline Replicated<T>::~Replicated()
{
    cout << "Number of multiplications: " << counter << endl;
}

template<class T>
void Replicated<T>::muls(const vector<int>& reg,
        SubProcessor<Share<T> >& proc, ReplicatedMC<T>& MC, int size)
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
            auto& x = proc.S[reg[3 * i + 1] + j].get_share();
            auto& y = proc.S[reg[3 * i + 2] + j].get_share();
            auto add_share = x[0] * y.sum() + x[1] * y[0];
            auto& result = results[i * size + j];
            result.randomize_to_sum(add_share, proc.Proc.secure_prng);
            for (int k = 0; k < 2; k++)
                result[k].pack(os[k]);
        }
    proc.P.send_relative(os);
    proc.P.receive_relative(os);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < size; j++)
        {
            auto& result = results[i * size + j];
            for (int k = 0; k < 2; k++)
            {
                typename T::value_type tmp;
                tmp.unpack(os[k]);
                result[k] += tmp;
            }
            proc.S[reg[3 * i] + j].set_share(result);
        }

    counter += n * size;
}

template class Replicated<FixedVec<Integer, 2> >;
