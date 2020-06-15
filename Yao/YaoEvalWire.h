/*
 * YaoEvalWire.h
 *
 */

#ifndef YAO_YAOEVALWIRE_H_
#define YAO_YAOEVALWIRE_H_

#include "BMR/Key.h"
#include "BMR/Gate.h"
#include "BMR/Register.h"
#include "Processor/DummyProtocol.h"
#include "config.h"
#include "YaoWire.h"

class YaoEvaluator;

class YaoEvalWire : public YaoWire
{
public:
	typedef YaoEvaluator Party;

	static string name() { return "YaoEvalWire"; }

	typedef ostream& out_type;
	static ostream& out;

	static YaoEvalWire new_reg() { return {}; }

	static void andrs(GC::Processor<GC::Secret<YaoEvalWire>>& processor,
			const vector<int>& args)
	{
		and_<true>(processor, args);
	}
	static void ands(GC::Processor<GC::Secret<YaoEvalWire>>& processor,
			const vector<int>& args)
	{
		and_<false>(processor, args);
	}
	template<bool repeat>
	static void and_(GC::Processor<GC::Secret<YaoEvalWire>>& processor,
			const vector<int>& args);
	template<bool repeat>
	static void and_singlethread(
			GC::Processor<GC::Secret<YaoEvalWire>>& processor,
			const vector<int>& args, int total_ands);
	static void and_(GC::Memory<GC::Secret<YaoEvalWire>>& S,
			const vector<int>& args, size_t start, size_t end,
			size_t total_ands, YaoGate* gate, long& counter, PRNG& prng,
			map<string, Timer>& timers, bool repeat, YaoEvaluator& garbler);

	static void inputb(GC::Processor<GC::Secret<YaoEvalWire>>& processor,
			const vector<int>& args);

	static void convcbit(Integer& dest, const GC::Clear& source,
			GC::Processor<GC::Secret<YaoEvalWire>>&);

	void set(const Key& key);
	void set(Key key, bool external);

	const Key& key() const
	{
		return key_;
	}

	bool external() const
	{
		return key_.get_signal();
	}

	void random();
	void public_input(bool value);
	void op(const YaoEvalWire& left, const YaoEvalWire& right, Function func);
	bool get_output();
};

#endif /* YAO_YAOEVALWIRE_H_ */
