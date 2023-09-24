/*
 * RmfeBeaver.cpp
 *
 */

#ifndef PROTOCOLS_RMFE_BEAVER_HPP_
#define PROTOCOLS_RMFE_BEAVER_HPP_

#include "RmfeBeaver.h"

#include "Replicated.hpp"
#include "Tools/mpdz_ntl_types.h"
#include "Tools/debug.h"

#include <array>

// template<class T>
// typename T::Protocol Beaver<T>::branch()
// {
//     typename T::Protocol res(P);
//     res.prep = prep;
//     res.MC = MC;
//     res.init_mul();
//     return res;
// }

template<class T>
void RmfeBeaver<T>::setup(Player& P) {
    BinaryProtocolThreadInit<TinyOTShare>::setup(P);
    setup_rmfe();
    setup_mfe();
}

template<class T>
void RmfeBeaver<T>::teardown() {
    BinaryProtocolThreadInit<TinyOTShare>::teardown();

    teardown_rmfe();
    teardown_mfe();
}

template<class T>
void RmfeBeaver<T>::setup_rmfe() {
    // Setup RMFE
    if (Gf2RMFE::has_singleton())
        throw runtime_error("Can only setup RMFE once");
    auto rmfe = get_composite_gf2_rmfe_type2(2, 6);
    Gf2RMFE::set_singleton(std::move(rmfe));
}

template<class T>
void RmfeBeaver<T>::teardown_rmfe() {
    if (!Gf2RMFE::has_singleton())
        return;
    Gf2RMFE::reset_singleton();
}

template<class T>
void RmfeBeaver<T>::setup_mfe() {
    // Setup MFE
    if (Gf2MFE::has_singleton())
        throw runtime_error("Can only setup MFE once");
    auto mfe = get_double_composite_gf2_mfe(2, 3, 8);
    Gf2MFE::set_singleton(std::move(mfe));
}

template<class T>
void RmfeBeaver<T>::teardown_mfe() {
    if (!Gf2MFE::has_singleton())
        return;
    Gf2MFE::reset_singleton();
}

template<class T>
void RmfeBeaver<T>::init(Preprocessing<T>& prep, typename T::MAC_Check& MC)
{
    this->prep = &prep;
    this->MC = &MC;
}

template<class T>
void RmfeBeaver<T>::init_mul()
{
    assert(this->prep);
    assert(this->MC);
    shares.clear();
    opened.clear();
    quintuples.clear();
    lengths.clear();
}

template<class T>
void RmfeBeaver<T>::prepare_mul(const T& x, const T& y, int n)
{
    (void) n;
    quintuples.push_back({{}});
    auto& quintuple = quintuples.back();
    quintuple = prep->get_quintuple(n);

    shares.push_back(x - quintuple[0]);
    shares.push_back(y - quintuple[1]);
    lengths.push_back(n);
}

template<class T>
void RmfeBeaver<T>::exchange()
{
    assert(shares.size() == 2 * lengths.size());
    MC->init_open(P, shares.size());
    for (size_t i = 0; i < shares.size(); i++) {
        MC->prepare_open(shares[i], lengths[i / 2]);
    }
    MC->exchange(P);
    for (size_t i = 0; i < shares.size(); i++)
        opened.push_back(MC->finalize_raw());
    it = opened.begin();
    quintuple = quintuples.begin();
}

template<class T>
void RmfeBeaver<T>::start_exchange()
{
    MC->POpen_Begin(opened, shares, P);
}

template<class T>
void RmfeBeaver<T>::stop_exchange()
{
    MC->POpen_End(opened, shares, P);
    it = opened.begin();
    quintuple = quintuples.begin();
}

template<class T>
T RmfeBeaver<T>::finalize_mul(int n)
{
    (void) n;
    typename T::open_type masked[2], norm_masked[2];
    T& tmp = (*quintuple)[2];
    NTL::GF2X ntl_tmp;

    for (int k = 0; k < 2; k++)
    {
        masked[k] = *it++;
    }
    conv(ntl_tmp, masked[0]);
    conv(norm_masked[0], Gf2RMFE::s().tau(ntl_tmp));
    conv(ntl_tmp, masked[1]);
    conv(norm_masked[1], Gf2RMFE::s().tau(ntl_tmp));

    tmp += (norm_masked[0] * (*quintuple)[4]);
    tmp += ((*quintuple)[3] * norm_masked[1]);
    tmp += T::constant(norm_masked[0] * norm_masked[1], P.my_num(), MC->get_alphai());
    quintuple++;

    return tmp;
}

template<class T>
void RmfeBeaver<T>::init_mul_constant()
{
    assert(this->prep);
    assert(this->MC);
    shares.clear();
    opened.clear();
    normals.clear();
    constants.clear();
    lengths.clear();
}

template<class T>
void RmfeBeaver<T>::prepare_mul_constant(const T& x, const typename T::clear& y, int n) {
    (void) n;
    typename T::open_type y_(y);
    normals.push_back({});
    auto& normal = normals.back();
    normal = prep->get_normal();

    shares.push_back(x - normal[0]);
    constants.push_back(y_);
}

template<class T>
void RmfeBeaver<T>::exchange_mul_constant()
{
    MC->init_open(P, shares.size());
    for (size_t i = 0; i < shares.size(); i++) {
        MC->prepare_open(shares[i]);
    }
    MC->exchange(P);
    for (size_t i = 0; i < shares.size(); i++)
        opened.push_back(MC->finalize_raw());
    it = opened.begin();
    normal = normals.begin();
    constant = constants.begin();
}

template<class T>
T RmfeBeaver<T>::finalize_mul_constant(int n)
{
    (void) n;
    typename T::open_type masked = *it++;
    T norm_masked_T = T::constant(T::open_type::tau(masked), P.my_num(), MC->get_alphai());
    T tmp = ((*normal)[1] + norm_masked_T) * (*constant);

    normal++;
    constant++;

    return tmp;
}


template<class T>
void RmfeBeaver<T>::check()
{
    assert(MC);
    MC->Check(P);
}

#endif
