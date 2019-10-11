/*
 * MalicousRepParty.cpp
 *
 */

#ifndef GC_SHARETHREAD_HPP_
#define GC_SHARETHREAD_HPP_

#include <GC/ShareThread.h>
#include "Protocols/MaliciousRepMC.h"
#include "Math/Setup.h"

#include "Processor/Data_Files.hpp"

namespace GC
{

template<class T>
ShareThread<T>::ShareThread(int i,
        ThreadMaster<T>& master) :
        Thread<T>(i, master), usage(master.N.num_players()), DataF(
                master.opts.live_prep ?
                        *(Preprocessing<T>*) new typename T::LivePrep(usage,
                                *this) :
                        *(Preprocessing<T>*) new Sub_Data_Files<T>(master.N,
                                get_prep_dir(master.N.num_players(), 128, 128),
                                usage))
{
}

template<class T>
ShareThread<T>::~ShareThread()
{
    delete &DataF;
}

template<class T>
void ShareThread<T>::pre_run()
{
    if (singleton)
        throw runtime_error("there can only be one");
    singleton = this;
    assert(this->protocol != 0);
    DataF.set_protocol(*this->protocol);
}

template<class T>
void ShareThread<T>::post_run()
{
#ifndef INSECURE
    cerr << "Removing used pre-processed data" << endl;
    DataF.prune();
#endif
}

template<class T>
void ShareThread<T>::and_(Processor<T>& processor,
        const vector<int>& args, bool repeat)
{
    auto& protocol = this->protocol;
    processor.check_args(args, 4);
    protocol->init_mul(DataF, *this->MC);
    for (size_t i = 0; i < args.size(); i += 4)
    {
        int n_bits = args[i];
        int left = args[i + 2];
        int right = args[i + 3];
        T y_ext;
        if (repeat)
            y_ext = processor.S[right].extend_bit();
        else
            y_ext = processor.S[right];
        protocol->prepare_mul(processor.S[left].mask(n_bits), y_ext.mask(n_bits), n_bits);
    }

    protocol->exchange();

    for (size_t i = 0; i < args.size(); i += 4)
    {
        int n_bits = args[i];
        int out = args[i + 1];
        processor.S[out] = protocol->finalize_mul(n_bits);
    }
}

} /* namespace GC */

#endif
