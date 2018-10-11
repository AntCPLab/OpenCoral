/*
 * YaoSimulator.cpp
 *
 */

#include "YaoSimulator.h"

YaoSimulator::YaoSimulator(string progname) : YaoEvaluator(progname), YaoGarbler(progname)
{
}

void YaoSimulator::run()
{
	YaoGarbler::run();
	YaoEvaluator::output_masks = YaoGarbler::output_masks;
	YaoEvaluator::gates = YaoGarbler::gates;
	YaoEvaluator::run();
}
