/*
 * config.h
 *
 */

#ifndef YAO_CONFIG_H_
#define YAO_CONFIG_H_

//#define DEBUG

//#define CHECK_BUFFER

#define HALF_GATES

class YaoFullGate;
class YaoHalfGate;

#ifdef HALF_GATES
typedef YaoHalfGate YaoGate;
#else
typedef YaoFullGate YaoGate;
#endif

#endif /* YAO_CONFIG_H_ */
