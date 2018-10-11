/*
 * YaoWire.h
 *
 */

#ifndef YAO_YAOGARBLEWIRE_H_
#define YAO_YAOGARBLEWIRE_H_

#include "BMR/Key.h"
#include "BMR/Register.h"
#include "GC/Processor.h"

class YaoGate;

class YaoGarbleWire : public Phase
{
public:
	static string name() { return "YaoGarbleWire"; }

	Key key;
	bool mask;

	static YaoGarbleWire new_reg() { return {}; }

	static void andrs(GC::Processor<GC::Secret<YaoGarbleWire>>& processor,
			const vector<int>& args);
	static void andrs_multithread(
			GC::Processor<GC::Secret<YaoGarbleWire>>& processor,
			const vector<int>& args);
	static void andrs_singlethread(
			GC::Processor<GC::Secret<YaoGarbleWire>>& processor,
			const vector<int>& args);
	static void andrs(GC::Memory<GC::Secret<YaoGarbleWire>>& S,
			const vector<int>& args, size_t start, size_t end,
			size_t total_ands, YaoGate* gate, long& counter, PRNG& prng,
			map<string, Timer>& timers);

	void randomize(PRNG& prng);

	void random();
	void public_input(bool value);
	void op(const YaoGarbleWire& left, const YaoGarbleWire& right, Function func);
	void XOR(const YaoGarbleWire& left, const YaoGarbleWire& right);
	char get_output();
};

#endif /* YAO_YAOGARBLEWIRE_H_ */
