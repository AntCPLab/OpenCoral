/*
 * config.h
 *
 */

#ifndef PROCESSOR_CONFIG_H_
#define PROCESSOR_CONFIG_H_

#include "Math/Share.h"
#include "Math/Rep3Share.h"

#ifdef REPLICATED
#error REPLICATED flag is obsolete
#endif

typedef Share<gfp> sgfp;

#endif /* PROCESSOR_CONFIG_H_ */
