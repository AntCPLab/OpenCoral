/*
 * YaoWire.h
 *
 */

#ifndef YAO_YAOGARBLEWIRE_H_
#define YAO_YAOGARBLEWIRE_H_

#include "BMR/Key.h"
#include "BMR/Register.h"
#include "config.h"
#include "YaoWire.h"

#include <map>

class YaoGarbler;

class YaoGarbleWire : public YaoWire
{
public:
	typedef YaoGarbler Party;

	static string name() { return "YaoGarbleWire"; }

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

	static void convcbit(Integer& dest, const GC::Clear& source,
			GC::Processor<GC::Secret<YaoGarbleWire>>&);

	void randomize(PRNG& prng);
	void set(Key key, bool mask);

	Key full_key() const
	{
	    return key_;
	}

	void set_full_key(Key key)
	{
	    key_ = key;
	}

	Key key() const
	{
		Key res = key_;
		res.set_signal(0);
		return res;
	}

	bool mask() const
	{
		return key_.get_signal();
	}

	void random();
	void public_input(bool value);
	void op(const YaoGarbleWire& left, const YaoGarbleWire& right, Function func);
	char get_output();
};

inline void YaoGarbleWire::randomize(PRNG& prng)
{
    key_ = prng.get_doubleword();
#ifdef DEBUG
    //key = YaoGarbler::s().counter << 1;
#endif
}

inline void YaoGarbleWire::set(Key key, bool mask)
{
    key.set_signal(mask);
    this->key_ = key;
    assert(key.get_signal() == mask);
}

#endif /* YAO_YAOGARBLEWIRE_H_ */
