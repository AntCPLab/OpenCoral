/*
 * FixInput.cpp
 *
 */

#include "FixInput.h"

const char* FixInput::NAME = "real number";

void FixInput::read(std::istream& in, const int* params)
{
    mpf_class x;
    in >> x;
    items[0] = x << *params;
}
