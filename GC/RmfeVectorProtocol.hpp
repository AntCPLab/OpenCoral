/*
 * RmfeVectorProtocol.cpp
 *
 */

#ifndef GC_RMFEVECTORPROTOCOL_HPP_
#define GC_RMFEVECTORPROTOCOL_HPP_

#include "RmfeVectorProtocol.h"

namespace GC
{

template<class T>
RmfeVectorProtocol<T>::RmfeVectorProtocol(Player& P) :
        part_protocol(P), P(P)
{
}

template<class T>
void RmfeVectorProtocol<T>::init(Preprocessing<T>& prep,
        typename T::MAC_Check& MC)
{
    part_protocol.init(prep.get_part(), MC.get_part_MC());
}

template<class T>
void RmfeVectorProtocol<T>::init_mul()
{
    part_protocol.init_mul();
}

template<class T>
void RmfeVectorProtocol<T>::prepare_mul(const T& x,
        const T& y, int n)
{
    (void) n;
    cout << "x: " << x.get_regs().size() << endl;
    cout << "y: " << y.get_regs().size() << endl;
    if (x.get_regs().size() != y.get_regs().size())
        throw runtime_error("RmfeVectorProtocol: Incompatible operands for prepare_mul");
    int n_parts = x.get_regs().size();
    for (int i = 0; i < n_parts; i++)
        part_protocol.prepare_mul(x.get_reg(i), y.get_reg(i));
    mul_sizes.push_back(n_parts);
}

template<class T>
void RmfeVectorProtocol<T>::exchange()
{
    part_protocol.exchange();
}

template<class T>
T RmfeVectorProtocol<T>::finalize_mul(int n)
{
    T res;
    finalize_mult(res, n);
    return res;
}

template<class T>
void RmfeVectorProtocol<T>::finalize_mult(T& res, int n)
{
    res.resize_regs(mul_sizes.front());
    for (int i = 0; i < mul_sizes.front(); i++)
        res.get_reg(i) = part_protocol.finalize_mul(1);
    mul_sizes.pop_front();
}

} /* namespace GC */

#endif
