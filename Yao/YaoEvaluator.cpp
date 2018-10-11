/*
 * YaoEvaluator.cpp
 *
 */

#include "YaoEvaluator.h"

thread_local YaoEvaluator* YaoEvaluator::singleton = 0;

YaoEvaluator::YaoEvaluator(int thread_num, YaoEvalMaster& master) :
		Thread<GC::Secret<YaoEvalWire>>(thread_num, master.machine, master.N),
		master(master),
		player(N, 0, thread_num << 24),
		ot_ext(OTExtensionWithMatrix::setup(player, {}, RECEIVER, true))
{
	set_n_program_threads(master.machine.nthreads);
}

void YaoEvaluator::pre_run()
{
	if (not continous())
		receive_to_store(*P);
}

void YaoEvaluator::run(GC::Program<GC::Secret<YaoEvalWire>>& program)
{
	singleton = this;

	if (continous())
		run(program, *P);
	else
	{
		run_from_store(program);
	}
}

void YaoEvaluator::run(GC::Program<GC::Secret<YaoEvalWire>>& program, Player& P)
{
	do
		receive(P);
	while(GC::DONE_BREAK != program.execute(processor, -1));
}

void YaoEvaluator::run_from_store(GC::Program<GC::Secret<YaoEvalWire>>& program)
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
