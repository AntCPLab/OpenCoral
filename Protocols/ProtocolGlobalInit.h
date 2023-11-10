/*
 * ProtocolGlobalInit.h
 *
 */

#ifndef PROTOCOLS_PROTOCOLGLOBALINIT_H_
#define PROTOCOLS_PROTOCOLGLOBALINIT_H_

#include <atomic>
#include <thread>
#include "ProtocolSetup.h"
#include "ProtocolSet.h"


template<class T>
class BinaryProtocolThreadInit {
    static std::atomic_flag setup_initialized;
    // This is a global setup process and should be called only once. E.g., the MAC_Check::setup(P) might contain 
    // the static `coordinator` variable that should be set only once.
    static unique_ptr<BinaryProtocolSetup<T>> global_setup;
    // This is a per-thread set that should be called by each thread. E.g., it contains the ShareThread instance 
    // which should only be constructed once per thread.
    static thread_local unique_ptr<BinaryProtocolSet<T>> thread_set;

public:
    static void setup(Player& P, string directory = "") {
        if (!setup_initialized.test_and_set()) {
            global_setup = unique_ptr<BinaryProtocolSetup<T>>(new BinaryProtocolSetup<T>(P, directory));
        }
        // Possible that another thread is still initializing `global_setup`
        while (!global_setup) std::this_thread::sleep_for(std::chrono::microseconds(1000));
        thread_set = unique_ptr<BinaryProtocolSet<T>>(new BinaryProtocolSet<T>(P, *global_setup));
    }
    static void teardown() {
        global_setup.reset(nullptr); // [zico] Double check whether this would cause race condition in multi threading
        thread_set.reset(nullptr);
    }
};

template<class T>
std::atomic_flag BinaryProtocolThreadInit<T>::setup_initialized = ATOMIC_FLAG_INIT;

template<class T>
unique_ptr<BinaryProtocolSetup<T>> BinaryProtocolThreadInit<T>::global_setup(nullptr);

template<class T>
thread_local unique_ptr<BinaryProtocolSet<T>> BinaryProtocolThreadInit<T>::thread_set(nullptr);


template<class T>
class MixedProtocolThreadInit {

public:
    static std::atomic_flag setup_initialized;
    // This is a global setup process and should be called only once. E.g., the MAC_Check::setup(P) might contain 
    // the static `coordinator` variable that should be set only once.
    static unique_ptr<MixedProtocolSetup<T>> global_setup;
    // This is a per-thread set that should be called by each thread. E.g., it contains the ShareThread instance 
    // which should only be constructed once per thread.
    static thread_local unique_ptr<MixedProtocolSet<T>> thread_set;

    static void setup(Player& P, int prime_length = 0, string directory = "", bool write_mac_key = false) {
        if (!setup_initialized.test_and_set()) {
            global_setup = unique_ptr<MixedProtocolSetup<T>>(new MixedProtocolSetup<T>(P, prime_length, directory, write_mac_key));
        }
        // Possible that another thread is still initializing `global_setup`
        while (!global_setup) std::this_thread::sleep_for(std::chrono::microseconds(1000));
        thread_set = unique_ptr<MixedProtocolSet<T>>(new MixedProtocolSet<T>(P, *global_setup));
    }
    static void teardown() {
        global_setup.reset(nullptr); // [zico] Double check whether this would cause race condition in multi threading
        thread_set.reset(nullptr);
    }
};

template<class T>
std::atomic_flag MixedProtocolThreadInit<T>::setup_initialized = ATOMIC_FLAG_INIT;

template<class T>
unique_ptr<MixedProtocolSetup<T>> MixedProtocolThreadInit<T>::global_setup(nullptr);

template<class T>
thread_local unique_ptr<MixedProtocolSet<T>> MixedProtocolThreadInit<T>::thread_set(nullptr);



#endif /* PROTOCOLS_PROTOCOLGLOBALINIT_H_ */
