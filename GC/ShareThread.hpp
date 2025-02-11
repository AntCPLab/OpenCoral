/*
 * MalicousRepParty.cpp
 *
 */

#ifndef GC_SHARETHREAD_HPP_
#define GC_SHARETHREAD_HPP_

#include <GC/ShareThread.h>
#include "GC/ShareParty.h"
#include "BitPrepFiles.h"
#include "Math/Setup.h"

#include "Processor/Data_Files.hpp"

namespace GC
{

template<class T>
StandaloneShareThread<T>::StandaloneShareThread(int i, ThreadMaster<T>& master) :
        ShareThread<T>(*Preprocessing<T>::get_new(master.opts.live_prep,
                master.N, usage)),
        Thread<T>(i, master)
{
}

template<class T>
StandaloneShareThread<T>::~StandaloneShareThread()
{
    delete &this->DataF;
}

template<class T>
ShareThread<T>::ShareThread(Preprocessing<T>& prep) :
    P(0), MC(0), protocol(0), DataF(prep)
{
}

template<class T>
ShareThread<T>::ShareThread(Preprocessing<T>& prep, Player& P,
        typename T::mac_key_type mac_key) :
        ShareThread(prep)
{
    pre_run(P, mac_key);
}

template<class T>
ShareThread<T>::~ShareThread()
{
    if (MC)
        delete MC;
    if (protocol)
        delete protocol;
}

template<class T>
void ShareThread<T>::pre_run(Player& P, typename T::mac_key_type mac_key)
{
    this->P = &P;
    if (singleton)
        throw runtime_error("there can only be one ShareThread: " + T::type_string());
    singleton = this;
    protocol = new typename T::Protocol(*this->P);
    MC = this->new_mc(mac_key);
    DataF.set_protocol(*this->protocol);
    this->protocol->init(DataF, *MC);
}

template<class T>
void StandaloneShareThread<T>::pre_run()
{
    ShareThread<T>::pre_run(*Thread<T>::P, ShareParty<T>::s().mac_key);
    usage.set_num_players(Thread<T>::P->num_players());
}

template<class T>
void ShareThread<T>::post_run()
{
    check();
}

template<class T>
void ShareThread<T>::check()
{
    protocol->check();
    MC->Check(*this->P);
}

template<class T>
void ShareThread<T>::and_(Processor<T>& processor,
        const vector<int>& args, bool repeat)
{
    auto& protocol = this->protocol;

    if (protocol->buffer_size_per_round() > 0) {
        buffering_and_(processor, args, repeat);
        return;
    }

    processor.check_args(args, 4);
    protocol->init_mul();
    T x_ext, y_ext;
    for (size_t i = 0; i < args.size(); i += 4)
    {
        int n_bits = args[i];
        int left = args[i + 2];
        int right = args[i + 3];
        for (int j = 0; j < DIV_CEIL(n_bits, T::default_length); j++)
        {
            int n = min(T::default_length, n_bits - j * T::default_length);
            if (repeat)
                processor.S[right].extend_bit(y_ext, n);
            else
                processor.S[right + j].mask(y_ext, n);
            processor.S[left + j].mask(x_ext, n);
            protocol->prepare_mult(x_ext, y_ext, n, repeat);
        }
    }

    protocol->exchange();

    for (size_t i = 0; i < args.size(); i += 4)
    {
        int n_bits = args[i];
        int out = args[i + 1];
        for (int j = 0; j < DIV_CEIL(n_bits, T::default_length); j++)
        {
            int n = min(T::default_length, n_bits - j * T::default_length);
            auto& res = processor.S[out + j];
            protocol->finalize_mult(res, n);
            res.mask(res, n);
        }
    }
}

template<class T>
void ShareThread<T>::check_buffering_and_(int& ii, int& jj, int i, int j, 
    Processor<T>& processor, const vector<int>& args) {
    typename T::Protocol& protocol = *(this->protocol);
    if (protocol.get_buffer_size() >= protocol.buffer_size_per_round()) {
        protocol.exchange();
        int ii_ = ii, jj_ = jj;
        if (ii_ == i) {
            int n_bits = args[ii_];
            int out = args[ii_ + 1];
            for (; jj_ <= j; jj_++) {
                int n = min(T::default_length, n_bits - jj_ * T::default_length);
                auto& res = processor.S[out + jj_];
                protocol.finalize_mult(res, n);
                res.mask(res, n);
            }
        }
        else {
            int n_bits = args[ii_];
            int out = args[ii_ + 1];
            for (; jj_ < DIV_CEIL(n_bits, T::default_length); jj_++) {
                int n = min(T::default_length, n_bits - jj_ * T::default_length);
                auto& res = processor.S[out + jj_];
                protocol.finalize_mult(res, n);
                res.mask(res, n);
            }
            for (ii_ += 4; ii_ < i; ii_ += 4) {
                int n_bits = args[ii_];
                int out = args[ii_ + 1];
                for (jj_ = 0; jj_ < DIV_CEIL(n_bits, T::default_length); jj_++) {
                    int n = min(T::default_length, n_bits - jj_ * T::default_length);
                    auto& res = processor.S[out + jj_];
                    protocol.finalize_mult(res, n);
                    res.mask(res, n);
                }
            }
            n_bits = args[ii_];
            out = args[ii_ + 1];
            for (jj_ = 0; jj_ <= j; jj_++) {
                int n = min(T::default_length, n_bits - jj_ * T::default_length);
                auto& res = processor.S[out + jj_];
                protocol.finalize_mult(res, n);
                res.mask(res, n);
            }
        }
        protocol.init_mul();
        ii = i;
        jj = j + 1;
    }
}

template<class T>
void ShareThread<T>::buffering_and_(Processor<T>& processor,
        const vector<int>& args, bool repeat)
{
    auto& protocol = this->protocol;
    processor.check_args(args, 4);

    int ii = 0, jj = 0;
    protocol->init_mul();
    T x_ext, y_ext;
    for (size_t i = 0; i < args.size(); i += 4)
    {
        int n_bits = args[i];
        int left = args[i + 2];
        int right = args[i + 3];
        for (int j = 0; j < DIV_CEIL(n_bits, T::default_length); j++)
        {
            int n = min(T::default_length, n_bits - j * T::default_length);
            if (repeat)
                processor.S[right].extend_bit(y_ext, n);
            else
                processor.S[right + j].mask(y_ext, n);
            processor.S[left + j].mask(x_ext, n);
            protocol->prepare_mult(x_ext, y_ext, n, repeat);

            check_buffering_and_(ii, jj, i, j, processor, args);
        }
    }

    protocol->exchange();

    int n_bits = args[ii];
    int out = args[ii + 1];
    for (; jj < DIV_CEIL(n_bits, T::default_length); jj++) {
        int n = min(T::default_length, n_bits - jj * T::default_length);
        auto& res = processor.S[out + jj];
        protocol->finalize_mult(res, n);
        res.mask(res, n);
    }
    for (ii += 4; ii < (int) args.size(); ii += 4) {
        int n_bits = args[ii];
        int out = args[ii + 1];
        for (jj = 0; jj < DIV_CEIL(n_bits, T::default_length); jj++) {
            int n = min(T::default_length, n_bits - jj * T::default_length);
            auto& res = processor.S[out + jj];
            protocol->finalize_mult(res, n);
            res.mask(res, n);
        }
    }
}

template<class T>
void ShareThread<T>::andrsvec(Processor<T>& processor, const vector<int>& args)
{
    int N_BITS = T::default_length;
    auto& protocol = this->protocol;
    assert(protocol);
    protocol->init_mul();
    auto it = args.begin();
    T x_ext, y_ext;
    while (it < args.end())
    {
        // [zico] The logic of -3 then / 2: 
        // This is how the args are storing things:
        // (#total_args)(size){n result args}(1 right arg){n left args}
        // where `#total_args` includes the total number of arguments, including itself.
        // Hence n = (#total_args - 3) / 2, the '3' represents the `int` for #total_args, size, and 1 right arg
        int n_args = (*it++ - 3) / 2;  
        int size = *it++;
        it += n_args;  // Skip the result part
        int base = *it++;
        assert(n_args <= N_BITS);
        for (int i = 0; i < size; i += N_BITS)
        {
            int n_ops = min(N_BITS, size - i);
            for (int j = 0; j < n_args; j++)
            {
                processor.S.at(*(it + j) + i / N_BITS).mask(x_ext, n_ops);
                processor.S.at(base + i / N_BITS).mask(y_ext, n_ops);
                protocol->prepare_mul(x_ext, y_ext, n_ops);
            }
        }
        it += n_args;
    }

    protocol->exchange();

    it = args.begin();
    while (it < args.end())
    {
        int n_args = (*it++ - 3) / 2;
        int size = *it++;
        for (int i = 0; i < size; i += N_BITS)
        {
            int n_ops = min(N_BITS, size - i);
            for (int j = 0; j < n_args; j++)
                protocol->finalize_mul(n_ops).mask(
                        processor.S.at(*(it + j) + i / N_BITS), n_ops);
        }
        it += 2 * n_args + 1;
    }
}

template<class T>
void ShareThread<T>::xors(Processor<T>& processor, const vector<int>& args)
{
    processor.check_args(args, 4);
    for (size_t i = 0; i < args.size(); i += 4)
    {
        int n_bits = args[i];
        int out = args[i + 1];
        int left = args[i + 2];
        int right = args[i + 3];
        if (n_bits == 1)
            processor.S[out].xor_(1, processor.S[left], processor.S[right]);
        else
            for (int j = 0; j < DIV_CEIL(n_bits, T::default_length); j++)
            {
                int n = min(T::default_length, n_bits - j * T::default_length);
                processor.S[out + j].xor_(n, processor.S[left + j],
                        processor.S[right + j]);
            }
    }
}

} /* namespace GC */

#endif
