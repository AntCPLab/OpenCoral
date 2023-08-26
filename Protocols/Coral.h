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
    // Spdz2kBShare setup for Coral prep
    static thread_local unique_ptr<BinaryProtocolThreadInit<GC::Spdz2kBShare<T::s>>> spdz2kb_init;
public:
    static void setup(Player& P) {
        spdz2kb_init.reset(new BinaryProtocolThreadInit<GC::Spdz2kBShare<T::s>>(P));
    }
    static void teardown() {
        spdz2kb_init.reset(nullptr);
    }

    Coral(Player& P) :
            SPDZ2k<T>(P)
    {
    }
};

template<class T>
thread_local unique_ptr<BinaryProtocolThreadInit<GC::Spdz2kBShare<T::s>>> Coral<T>::spdz2kb_init;

#endif /* PROTOCOLS_CORAL_H_ */
