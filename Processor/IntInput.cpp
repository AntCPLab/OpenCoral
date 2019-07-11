/*
 * IntInput.cpp
 *
 */

#include "IntInput.h"

const char* IntInput::NAME = "integer";

void IntInput::read(std::istream& in, const int* params)
{
    (void) params;
    in >> items[0];
}
