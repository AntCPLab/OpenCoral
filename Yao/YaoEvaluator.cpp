/*
 * YaoEvaluator.cpp
 *
 */

#include "YaoEvaluator.h"

YaoEvaluator* YaoEvaluator::singleton = 0;

YaoEvaluator::YaoEvaluator(string progname) : machine(MD), processor(machine)
{
	counter = 0;

	program.parse(progname + "-0");
	processor.reset(program);

	if (singleton)
		throw runtime_error("there can only be one");
	singleton = this;
}

void YaoEvaluator::run()
{
	while(GC::DONE_BREAK != program.execute(processor, -1))
		;
}

void YaoEvaluator::run(Player& P)
{
	do
		receive(P);
	while(GC::DONE_BREAK != program.execute(processor, -1));
}

void YaoEvaluator::run_from_store()
{
	machine.reset_timer();
	do
	{
		gates_store.pop(gates);
		output_masks_store.pop(output_masks);
	}
	while(GC::DONE_BREAK != program.execute(processor, -1));
}

void YaoEvaluator::receive(Player& P)
{
	P.receive_player(0, gates);
	P.receive_player(0, output_masks);
}

void YaoEvaluator::receive_to_store(Player& P)
{
	while (P.peek_long(0) != -1)
	{
		receive(P);
		gates_store.push(gates);
		output_masks_store.push(output_masks);
	}
	P.receive_long(0);
}
