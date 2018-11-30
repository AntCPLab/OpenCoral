/*
 * YaoEvaluator.h
 *
 */

#ifndef YAO_YAOEVALUATOR_H_
#define YAO_YAOEVALUATOR_H_

#include "YaoGate.h"
#include "YaoEvalMaster.h"
#include "YaoCommon.h"
#include "GC/Secret.h"
#include "GC/Thread.h"
#include "Tools/MMO.h"
#include "OT/OTExtensionWithMatrix.h"

class YaoEvaluator : public GC::Thread<Secret<YaoEvalWire>>, public YaoCommon
{
protected:
	static thread_local YaoEvaluator* singleton;

	ReceivedMsg gates;
	ReceivedMsgStore gates_store;

	YaoEvalMaster& master;

public:
	ReceivedMsg output_masks;
	ReceivedMsgStore output_masks_store;

	MMO mmo;

	TwoPartyPlayer player;
	OTExtensionWithMatrix ot_ext;

	static YaoEvaluator& s();

	YaoEvaluator(int thread_num, YaoEvalMaster& master);

	bool continuous() { return master.continuous and thread_num == 0; }

	void pre_run();
	void run(GC::Program<GC::Secret<YaoEvalWire>>& program);
	void run(GC::Program<GC::Secret<YaoEvalWire>>& program, Player& P);
	void run_from_store(GC::Program<GC::Secret<YaoEvalWire>>& program);
	bool receive(Player& P);
	void receive_to_store(Player& P);

	void load_gate(YaoGate& gate);

	long get_gate_id() { return gate_id(thread_num); }
};

inline void YaoEvaluator::load_gate(YaoGate& gate)
{
	gates.unserialize(gate);
}

inline YaoEvaluator& YaoEvaluator::s()
{
	if (singleton)
		return *singleton;
	else
		throw runtime_error("singleton unavailable");
}

#endif /* YAO_YAOEVALUATOR_H_ */
