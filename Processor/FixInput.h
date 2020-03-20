/*
 * FixInput.h
 *
 */

#ifndef PROCESSOR_FIXINPUT_H_
#define PROCESSOR_FIXINPUT_H_

#include <iostream>

#include "Math/bigint.h"
#include "Math/Integer.h"

class FixInput
{
public:
    const static int N_DEST = 1;
    const static int N_PARAM = 1;
    const static char* NAME;

    const static int TYPE = 1;

#ifdef LOW_PREC_INPUT
    Integer items[N_DEST];
#else
    bigint items[N_DEST];
#endif

    void read(std::istream& in, const int* params);
};

#endif /* PROCESSOR_FIXINPUT_H_ */
