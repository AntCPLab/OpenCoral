/*
 * YaoGarbler.h
 *
 */

#ifndef YAO_YAOGARBLER_H_
#define YAO_YAOGARBLER_H_

#include "YaoGarbleWire.h"
#include "YaoAndJob.h"
#include "Tools/random.h"
#include "Tools/MMO.h"
#include "GC/Secret.h"
#include "GC/Program.h"
#include "Networking/Player.h"
#include "sys/sysinfo.h"

class YaoGate;

class YaoGarbler
{
	friend class YaoGarbleWire;

protected:
	static YaoGarbler* singleton;

	Key delta;
	SendBuffer gates;

	GC::Program< GC::Secret<YaoGarbleWire> > program;
	GC::Machine< GC::Secret<YaoGarbleWire> > machine;
	GC::Processor< GC::Secret<YaoGarbleWire> > processor;
	GC::Memory<GC::Secret<YaoGarbleWire>::DynamicType> MD;

	int threshold;

	Timer and_timer;
	Timer and_proc_timer;
	Timer and_main_thread_timer;
	DoubleTimer and_prepare_timer;
	DoubleTimer and_wait_timer;

public:
	PRNG prng;
	SendBuffer output_masks;
	long counter;
	MMO mmo;

	YaoAndJob* and_jobs;

	map<string, Timer> timers;

	static YaoGarbler& s();

	YaoGarbler(string progname, int threshold = 1024);
	~YaoGarbler();
	void run();
	void run(Player& P);
	void send(Player& P);

	const Key& get_delta() { return delta; }
	void store_gate(const YaoGate& gate);

	int get_n_threads() { return get_nprocs(); }
	int get_threshold() { return threshold; }
};

inline YaoGarbler& YaoGarbler::s()
{
	if (singleton)
		return *singleton;
	else
		throw runtime_error("singleton unavailable");
}

#endif /* YAO_YAOGARBLER_H_ */
