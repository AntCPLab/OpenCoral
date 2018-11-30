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

class YaoEvalWire : public Phase
{
public:
	typedef DummyMC MC;

	static string name() { return "YaoEvalWire"; }

	typedef ostream& out_type;
	static ostream& out;

	bool external;
	Key key;

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

	static void inputb(GC::Processor<GC::Secret<YaoEvalWire>>& processor,
			const vector<int>& args);

	void set(const Key& key);
	void set(Key key, bool external);

	void random();
	void public_input(bool value);
	void op(const YaoEvalWire& left, const YaoEvalWire& right, Function func);
	void XOR(const YaoEvalWire& left, const YaoEvalWire& right);
	bool get_output();
};

#endif /* YAO_YAOEVALWIRE_H_ */
