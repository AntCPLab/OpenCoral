/*
 * Coral.h
 *
 */

#ifndef PROTOCOLS_CORAL_H_
#define PROTOCOLS_CORAL_H_

#include "SPDZ2k.h"

template<class T>
class Coral : public SPDZ2k<T>
{
public:
    Coral(Player& P) :
            SPDZ2k<T>(P)
    {
    }
};

#endif /* PROTOCOLS_CORAL_H_ */
