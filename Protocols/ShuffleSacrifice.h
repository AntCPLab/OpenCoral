/*
 * ShuffleSacrifice.h
 *
 */

#ifndef PROTOCOLS_SHUFFLESACRIFICE_H_
#define PROTOCOLS_SHUFFLESACRIFICE_H_

#include <vector>
#include <array>
using namespace std;

#include "Tools/FixedVector.h"
#include "edabit.h"

class Player;

template<class T> class LimitedPrep;

template<class T>
using dabit = pair<T, typename T::bit_type::part_type>;

template<class T>
class ShuffleSacrifice
{
    typedef typename T::bit_type::part_type BT;

    static const int B = 3;

public:
    static const int C = 3;

    static int minimum_n_inputs(int n_outputs = 1)
    {
        return max(n_outputs, minimum_n_outputs()) * B + C;
    }
    static int minimum_n_inputs_with_combining()
    {
        return minimum_n_inputs(B * minimum_n_outputs());
    }
    static int minimum_n_outputs()
    {
#ifdef INSECURE
        if (OnlineOptions::singleton.fake_batch)
        {
            cout << "FAKE FAKE FAKE" << endl;
            return 1 << 10;
        }
#endif
        return 1 << 20;
    }

    template<class U>
    void shuffle(vector<U>& items, Player& P);

    void triple_sacrifice(vector<array<T, 3>>& triples,
            vector<array<T, 3>>& check_triples, Player& P,
            typename T::MAC_Check& MC);
    void triple_combine(vector<array<T, 3>>& triples,
            vector<array<T, 3>>& to_combine, Player& P,
            typename T::MAC_Check& MC);

    void edabit_sacrifice(vector<edabit<T>>& output, vector<T>& sums,
            vector<vector<typename T::bit_type::part_type>>& bits, size_t n_bits,
            SubProcessor<T>& proc, bool strict = false, int player = -1,
            ThreadQueues* = 0);

    void edabit_sacrifice_buckets(vector<edabit<T>>& to_check, size_t n_bits,
            bool strict, int player, SubProcessor<T>& proc, int begin, int end,
            const void* supply = 0);

    void edabit_sacrifice_buckets(vector<edabit<T>>& to_check, size_t n_bits,
            bool strict, int player, SubProcessor<T>& proc, int begin, int end,
            LimitedPrep<BT>& personal_prep, const void* supply = 0);
};

#endif /* PROTOCOLS_SHUFFLESACRIFICE_H_ */
