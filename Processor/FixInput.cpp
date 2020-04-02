/*
 * FixInput.cpp
 *
 */

#include "FixInput.h"

template<>
void FixInput_<Integer>::read(std::istream& in, const int* params)
{
    double x;
    in >> x;
    items[0] = x * (1 << *params);
}

template<>
void FixInput_<bigint>::read(std::istream& in, const int* params)
{
    mpf_class x;
    in >> x;
    items[0] = x << *params;
}
