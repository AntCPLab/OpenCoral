/*
 * benchmarking.cpp
 *
 */

#include "benchmarking.h"

void insecure_fake()
{
#if defined(INSECURE) or defined(INSECURE_FAKE)
    cerr << "WARNING: insecure preprocessing" << endl;
#else
    insecure("preprocessing");
#endif
}
