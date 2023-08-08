/*
 * ProtocolGlobalInit.h
 *
 */

#ifndef PROTOCOLS_PROTOCOLGLOBALINIT_H_
#define PROTOCOLS_PROTOCOLGLOBALINIT_H_

#include "ProtocolSetup.h"
#include "ProtocolSet.h"

template<class T>
class BinaryProtocolGlobalInit {
    unique_ptr<BinaryProtocolSetup<T>> setup;
    unique_ptr<BinaryProtocolSet<T>> set;

    static thread_local bool initialized;
public:
    BinaryProtocolGlobalInit(Player& P) {
        if (initialized)
            return;
        setup = unique_ptr<BinaryProtocolSetup<T>>(new BinaryProtocolSetup<T>(P));
        set = unique_ptr<BinaryProtocolSet<T>>(new BinaryProtocolSet<T>(P, *setup));
        initialized = true;
    }
};

template<class T>
thread_local bool BinaryProtocolGlobalInit<T>::initialized = false;

#endif /* PROTOCOLS_PROTOCOLGLOBALINIT_H_ */
