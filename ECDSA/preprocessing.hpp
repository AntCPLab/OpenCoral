/*
 * preprocessing.hpp
 *
 */

#ifndef ECDSA_PREPROCESSING_HPP_
#define ECDSA_PREPROCESSING_HPP_

#include "P256Element.h"
#include "Processor/Data_Files.h"
#include "Protocols/ReplicatedPrep.h"
#include "Protocols/MaliciousShamirShare.h"

template<template<class U> class T>
class EcTuple
{
public:
    T<P256Element::Scalar> a;
    T<P256Element::Scalar> b;
    P256Element R;
};

template<template<class U> class T>
void preprocessing(vector<EcTuple<T>>& tuples, int buffer_size,
        T<P256Element::Scalar>& sk,
        SubProcessor<T<P256Element::Scalar>>& proc, bool prep_mul = true)
{
    Timer timer;
    timer.start();
    Player& P = proc.P;
    auto& prep = proc.DataF;
    size_t start = P.sent + prep.data_sent();
    auto& protocol = proc.protocol;
    auto& MCp = proc.MC;
    typedef T<typename P256Element::Scalar> pShare;
    typedef T<P256Element> cShare;
    vector<pShare> inv_ks;
    vector<cShare> secret_Rs;
    for (int i = 0; i < buffer_size; i++)
    {
        pShare a, a_inv;
        prep.get_two(DATA_INVERSE, a, a_inv);
        inv_ks.push_back(a_inv);
        secret_Rs.push_back(a);
    }
    if (prep_mul)
    {
        protocol.init_mul(&proc);
        for (int i = 0; i < buffer_size; i++)
            protocol.prepare_mul(inv_ks[i], sk);
        protocol.exchange();
    }
    else
        prep.buffer_triples();
    vector<P256Element> opened_Rs;
    typename cShare::MAC_Check MCc(MCp.get_alphai());
    MCc.POpen(opened_Rs, secret_Rs, P);
    MCc.Check(P);
    MCp.Check(P);
    for (int i = 0; i < buffer_size; i++)
    {
        tuples.push_back(
                { inv_ks[i], prep_mul ? protocol.finalize_mul() : pShare(),
                        opened_Rs[i] });
    }
    timer.stop();
    cout << "Generated " << buffer_size << " tuples in " << timer.elapsed()
            << " seconds, throughput " << buffer_size / timer.elapsed() << ", "
            << 1e-3 * (P.sent + prep.data_sent() - start) / buffer_size
            << " kbytes per tuple" << endl;
}

template<template<class U> class T>
void check(vector<EcTuple<T>>& tuples, T<P256Element::Scalar> sk,
    P256Element::Scalar alphai, Player& P)
{
    typename T<P256Element::Scalar>::MAC_Check MC(alphai);
    auto open_sk = MC.POpen(sk, P);
    for (auto& tuple : tuples)
    {
        auto inv_k = MC.POpen(tuple.a, P);
        auto k = inv_k;
        k.invert();
        assert(open_sk * inv_k == MC.POpen(tuple.b, P));
        assert(tuple.R == k);
    }
}

template<>
void ReplicatedPrep<Rep3Share<P256Element::Scalar>>::buffer_bits()
{
    throw not_implemented();
}

template<>
void ReplicatedPrep<ShamirShare<P256Element::Scalar>>::buffer_bits()
{
    throw not_implemented();
}

#endif /* ECDSA_PREPROCESSING_HPP_ */
