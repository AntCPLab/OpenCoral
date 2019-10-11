/*
 * FixInput.h
 *
 */

#ifndef PROCESSOR_FIXINPUT_H_
#define PROCESSOR_FIXINPUT_H_

#include <iostream>

#include "Math/bigint.h"

class FixInput
{
public:
    const static int N_DEST = 1;
    const static int N_PARAM = 1;
    const static char* NAME;

    const static int TYPE = 1;

    bigint items[N_DEST];

    void read(std::istream& in, const int* params);
};

#endif /* PROCESSOR_FIXINPUT_H_ */
