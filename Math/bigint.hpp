/*
 * bigint.hpp
 *
 */

#ifndef MATH_BIGINT_HPP_
#define MATH_BIGINT_HPP_

#include "bigint.h"
#include "Integer.h"

template<class T>
mpf_class bigint::get_float(T v, Integer exp, T z, T s)
{
    bigint tmp = v;
    mpf_class res = tmp;
    if (exp > 0)
        mpf_mul_2exp(res.get_mpf_t(), res.get_mpf_t(), exp.get());
    else
        mpf_div_2exp(res.get_mpf_t(), res.get_mpf_t(), -exp.get());
    if (z.is_one())
        res = 0;
    if (s.is_one())
    {
        res *= -1;
    }
    if (not z.is_bit() or not s.is_bit())
        throw Processor_Error("invalid floating point number");
    return res;
}

#endif /* MATH_BIGINT_HPP_ */
