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
class ShuffleSacrifice
{
    static const int B = 3;
    static const int C = 3;

public:
    static int minimum_n_inputs(int n_outputs = 1)
    {
        return max(n_outputs, minimum_n_outputs()) * B + C;
    }
    static int minimum_n_outputs()
    {
        return 1 << 20;
    }

    void triple_sacrifice(vector<array<T, 3>>& triples,
            vector<array<T, 3>>& check_triples, Player& P,
            typename T::MAC_Check& MC);
};

#endif /* PROTOCOLS_SHUFFLESACRIFICE_H_ */
