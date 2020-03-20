/*
 * FixInput.cpp
 *
 */

#include "FixInput.h"

const char* FixInput::NAME = "real number";

void FixInput::read(std::istream& in, const int* params)
{
#ifdef LOW_PREC_INPUT
    double x;
    in >> x;
    items[0] = x * (1 << *params);
#else
    mpf_class x;
    in >> x;
    items[0] = x << *params;
#endif
}
