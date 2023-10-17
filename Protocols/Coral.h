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
        // Use the same directory where arithmetic mac key is stored
        string directory = get_prep_sub_dir<T>(P.num_players());
        BinaryProtocolThreadInit<GC::Spdz2kBShare<T::s>>::setup(P, directory);
    }
    static void teardown() {
        BinaryProtocolThreadInit<GC::Spdz2kBShare<T::s>>::teardown();
    }

    Coral(Player& P) :
            SPDZ2k<T>(P)
    {
    }

    int buffer_size_per_round() {
        return 1000000;
    }
};

#endif /* PROTOCOLS_CORAL_H_ */
