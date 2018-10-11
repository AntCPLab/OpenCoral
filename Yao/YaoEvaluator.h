/*
 * YaoEvaluator.h
 *
 */

#ifndef YAO_YAOEVALUATOR_H_
#define YAO_YAOEVALUATOR_H_

#include "YaoGate.h"
#include "GC/Secret.h"
#include "GC/Program.h"
#include "GC/Machine.h"
#include "GC/Processor.h"
#include "GC/Memory.h"
#include "Tools/MMO.h"

class YaoEvaluator
{
protected:
	static YaoEvaluator* singleton;

	ReceivedMsg gates;
	ReceivedMsgStore gates_store;

	GC::Program< GC::Secret<YaoEvalWire> > program;
	GC::Machine< GC::Secret<YaoEvalWire> > machine;
	GC::Processor< GC::Secret<YaoEvalWire> > processor;
	GC::Memory<GC::Secret<YaoGarbleWire>::DynamicType> MD;

public:
	ReceivedMsg output_masks;
	ReceivedMsgStore output_masks_store;

	MMO mmo;

	long counter;

	static YaoEvaluator& s();

	YaoEvaluator(string progname);
	void run();
	void run(Player& P);
	void run_from_store();
	void receive(Player& P);
	void receive_to_store(Player& P);

	void load_gate(YaoGate& gate);
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
