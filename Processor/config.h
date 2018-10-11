/*
 * config.h
 *
 */

#ifndef PROCESSOR_CONFIG_H_
#define PROCESSOR_CONFIG_H_

#include "Math/Share.h"

//#define REPLICATED

#ifdef REPLICATED
typedef Share<FixedVec<Integer, 2> > sint;
#else
typedef Share<gfp> sint;
#endif

typedef Share<gf2n> sgf2n;

#endif /* PROCESSOR_CONFIG_H_ */
