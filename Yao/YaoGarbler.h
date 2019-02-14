/*
 * YaoGarbler.h
 *
 */

#ifndef YAO_YAOGARBLER_H_
#define YAO_YAOGARBLER_H_

#include "YaoGarbleWire.h"
#include "YaoAndJob.h"
#include "YaoGarbleMaster.h"
#include "YaoCommon.h"
#include "Tools/random.h"
#include "Tools/MMO.h"
#include "GC/Secret.h"
#include "Networking/Player.h"
#include "OT/OTExtensionWithMatrix.h"

#include <thread>

using namespace GC;

class YaoGate;

class YaoGarbler : public GC::Thread<GC::Secret<YaoGarbleWire>>, public YaoCommon
{
	friend class YaoGarbleWire;

protected:
	static thread_local YaoGarbler* singleton;

	YaoGarbleMaster& master;

	SendBuffer gates;

	Timer and_timer;
	Timer and_proc_timer;
	Timer and_main_thread_timer;
	DoubleTimer and_prepare_timer;
	DoubleTimer and_wait_timer;

public:
	PRNG prng;
	SendBuffer output_masks;
	MMO mmo;

	vector<YaoAndJob*> and_jobs;

	map<string, Timer> timers;

	TwoPartyPlayer player;
	OTExtensionWithMatrix ot_ext;

	deque<vector<Key>> receiver_input_keys;

	static YaoGarbler& s();

	YaoGarbler(int thread_num, YaoGarbleMaster& master);
	~YaoGarbler();
	void run(GC::Program<GC::Secret<YaoGarbleWire>>& program);
	void run(Player& P, bool continuous);
	void post_run();
	void send(Player& P);

	void process_receiver_inputs();

	const Key& get_delta() { return master.delta; }
	void store_gate(const YaoGate& gate);

	int get_n_worker_threads()
	{ return max(1u, thread::hardware_concurrency() / master.machine.nthreads); }
	int get_threshold() { return master.threshold; }

	long get_gate_id() { return gate_id(thread_num); }
};

inline YaoGarbler& YaoGarbler::s()
{
	if (singleton)
		return *singleton;
	else
		throw runtime_error("singleton unavailable");
}

#endif /* YAO_YAOGARBLER_H_ */
