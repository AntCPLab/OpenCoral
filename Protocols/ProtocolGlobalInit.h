/*
 * ProtocolGlobalInit.h
 *
 */

#ifndef PROTOCOLS_PROTOCOLGLOBALINIT_H_
#define PROTOCOLS_PROTOCOLGLOBALINIT_H_

#include <atomic>
#include "ProtocolSetup.h"
#include "ProtocolSet.h"

/**
 * This should be used as a pre-thread initialization. In other words,
 * 1. Each thread should call it only once, because the BinaryProtocolSet contains ShareThread instance which should only be
 * constructed once per thread.
 * 2. Should allow different threads to initialize, because the ShareThread instance contains a thread local singleton which
 * should be set independently for each thread.
*/
template<class T>
class BinaryProtocolThreadInit {
    unique_ptr<BinaryProtocolSetup<T>> setup;
    unique_ptr<BinaryProtocolSet<T>> set;

    // static std::atomic_flag initialized;
public:
    BinaryProtocolThreadInit(Player& P) {
        // if (initialized.test_and_set())
        //     return;
        setup = unique_ptr<BinaryProtocolSetup<T>>(new BinaryProtocolSetup<T>(P));
        set = unique_ptr<BinaryProtocolSet<T>>(new BinaryProtocolSet<T>(P, *setup));
    }
};

// template<class T>
// std::atomic_flag BinaryProtocolGlobalInit<T>::initialized = ATOMIC_FLAG_INIT;

#endif /* PROTOCOLS_PROTOCOLGLOBALINIT_H_ */
