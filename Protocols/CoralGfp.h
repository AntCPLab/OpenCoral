/*
 * CoralGfp.h
 *
 */

#ifndef PROTOCOLS_CORALGFP_H_
#define PROTOCOLS_CORALGFP_H_

#include "SPDZ.h"

template<class T>
class CoralGfp : public SPDZ<T>
{
public:

    CoralGfp(Player& P) :
            SPDZ<T>(P)
    {
    }

    int buffer_size_per_round() {
        return 1000000;
    }
};

#endif /* PROTOCOLS_CORALGFP_H_ */
