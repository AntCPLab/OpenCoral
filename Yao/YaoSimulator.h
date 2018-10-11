/*
 * YaoSimulator.h
 *
 */

#ifndef YAO_YAOSIMULATOR_H_
#define YAO_YAOSIMULATOR_H_

#include "YaoEvaluator.h"
#include "YaoGarbler.h"

class YaoSimulator : public YaoEvaluator, public YaoGarbler
{
public:
	YaoSimulator(string progname);
	void run();
};

#endif /* YAO_YAOSIMULATOR_H_ */
