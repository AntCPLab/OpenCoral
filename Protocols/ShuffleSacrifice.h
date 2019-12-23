/*
 * ShuffleSacrifice.h
 *
 */

#ifndef PROTOCOLS_SHUFFLESACRIFICE_H_
#define PROTOCOLS_SHUFFLESACRIFICE_H_

#include <vector>
#include <array>
using namespace std;

class Player;

template<class T>
using dabit = pair<T, typename T::bit_type::part_type>;

template<class T>
class ShuffleSacrifice
{
    static const int B = 3;
    static const int C = 3;

public:
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
#ifdef FAKE_BATCH
        cout << "FAKE FAKE FAKE" << endl;
        return 1 << 10;
#endif
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

    void dabit_sacrifice(vector<dabit<T>>& dabits,
            vector<dabit<T>>& check_dabits, SubProcessor<T>& proc);
};

#endif /* PROTOCOLS_SHUFFLESACRIFICE_H_ */
