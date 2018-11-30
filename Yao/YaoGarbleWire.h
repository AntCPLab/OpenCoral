/*
 * YaoWire.h
 *
 */

#ifndef YAO_YAOGARBLEWIRE_H_
#define YAO_YAOGARBLEWIRE_H_

#include "BMR/Key.h"
#include "BMR/Register.h"

#include <map>

class YaoGate;
class YaoGarbler;

class YaoGarbleWire : public Phase
{
public:
	static string name() { return "YaoGarbleWire"; }

	Key key;
	bool mask;

	static YaoGarbleWire new_reg() { return {}; }

	static void andrs(GC::Processor<GC::Secret<YaoGarbleWire>>& processor,
			const vector<int>& args)
	{
		and_(processor, args, true);
	}
	static void ands(GC::Processor<GC::Secret<YaoGarbleWire>>& processor,
			const vector<int>& args)
	{
		and_(processor, args, false);
	}

	static void and_(GC::Processor<GC::Secret<YaoGarbleWire>>& processor,
			const vector<int>& args, bool repeat);
	static void and_multithread(
			GC::Processor<GC::Secret<YaoGarbleWire>>& processor,
			const vector<int>& args, bool repeat);
	static void and_singlethread(
			GC::Processor<GC::Secret<YaoGarbleWire>>& processor,
			const vector<int>& args, bool repeat);
	static void and_(GC::Memory<GC::Secret<YaoGarbleWire>>& S,
			const vector<int>& args, size_t start, size_t end,
			size_t total_ands, YaoGate* gate, long& counter, PRNG& prng,
			map<string, Timer>& timers, bool repeat, YaoGarbler& garbler);

	static void inputb(GC::Processor<GC::Secret<YaoGarbleWire>>& processor,
			const vector<int>& args);

	void randomize(PRNG& prng);
	void set(Key key, bool mask);

	void random();
	void public_input(bool value);
	void op(const YaoGarbleWire& left, const YaoGarbleWire& right, Function func);
	void XOR(const YaoGarbleWire& left, const YaoGarbleWire& right);
	char get_output();
};

#endif /* YAO_YAOGARBLEWIRE_H_ */
