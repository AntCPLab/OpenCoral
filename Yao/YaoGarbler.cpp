/*
 * YaoGarbler.cpp
 *
 */

#include "YaoGarbler.h"
#include "YaoGate.h"

YaoGarbler* YaoGarbler::singleton = 0;

YaoGarbler::YaoGarbler(string progname, int threshold) :
		machine(MD), processor(machine), threshold(threshold),
		and_proc_timer(CLOCK_PROCESS_CPUTIME_ID),
		and_main_thread_timer(CLOCK_THREAD_CPUTIME_ID)
{
	prng.ReSeed();
	delta = prng.get_doubleword();
	delta.set_signal(1);
	counter = 0;
#ifdef DEBUG_DELTA
	delta = 1;
#endif

	program.parse(progname + "-0");
	processor.reset(program);

	and_jobs = new YaoAndJob[get_n_threads()];

	if (singleton)
		throw runtime_error("there can only be one");
	singleton = this;
}

YaoGarbler::~YaoGarbler()
{
	delete[] and_jobs;
#ifdef YAO_TIMINGS
	cout << "AND time: " << and_timer.elapsed() << endl;
	cout << "AND process timer: " << and_proc_timer.elapsed() << endl;
	cout << "AND main thread timer: " << and_main_thread_timer.elapsed() << endl;
	cout << "AND prepare timer: " << and_prepare_timer.elapsed() << endl;
	cout << "AND wait timer: " << and_wait_timer.elapsed() << endl;
	for (auto& x : timers)
		cout << x.first << " time:" << x.second.elapsed() << endl;
#endif
}

void YaoGarbler::run()
{
	while(GC::DONE_BREAK != program.execute(processor, -1))
		;
}

void YaoGarbler::run(Player& P)
{
	GC::BreakType b = GC::TIME_BREAK;
	while(GC::DONE_BREAK != b)
	{
		b = program.execute(processor, -1);
		send(P);
		gates.clear();
		output_masks.clear();
	}
}

void YaoGarbler::send(Player& P)
{
	P.send_to(1, gates, true);
	P.send_to(1, output_masks, true);
}
