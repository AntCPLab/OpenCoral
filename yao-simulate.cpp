/*
 * yao-simulate.cpp
 *
 */

#include "Yao/YaoSimulator.h"

int main(int argc, char** argv)
{
	if (argc < 1)
		throw exception();
	YaoSimulator(argv[1]).run();
}
