/*
 * Coral.h
 *
 */

#ifndef PROTOCOLS_CORAL_H_
#define PROTOCOLS_CORAL_H_

#include "SPDZ2k.h"
#include "GC/Spdz2kBShare.h"

template<class T>
class Coral : public SPDZ2k<T>
{
public:
    static void setup(Player& P) {
        BinaryProtocolThreadInit<GC::Spdz2kBShare<T::s>>::setup(P);
    }
    static void teardown() {
        BinaryProtocolThreadInit<GC::Spdz2kBShare<T::s>>::teardown();
    }

    Coral(Player& P) :
            SPDZ2k<T>(P)
    {
    }
};

#endif /* PROTOCOLS_CORAL_H_ */
