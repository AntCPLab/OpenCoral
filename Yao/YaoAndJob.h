/*
 * YaoAndJob.h
 *
 */

#ifndef YAO_YAOANDJOB_H_
#define YAO_YAOANDJOB_H_

#include "YaoGarbleWire.h"
#include "Tools/Worker.h"

class YaoGate;

class YaoAndJob
{
	GC::Memory< GC::Secret<YaoGarbleWire> >* S;
	const vector<int>* args;
	size_t start, end, n_gates;
	YaoGate* gate;
	long counter;
	PRNG prng;
	map<string, Timer> timers;

public:
	Worker<YaoAndJob> worker;

	YaoAndJob() : S(0), args(0), start(0), end(0), n_gates(0), gate(0),
			counter(0) { prng.ReSeed(); }

	~YaoAndJob()
	{
		for (auto& x : timers)
			cout << x.first << " time:" << x.second.elapsed() << endl;
	}

	void dispatch(GC::Memory<GC::Secret<YaoGarbleWire> >& S, const vector<int>& args,
			size_t start, size_t end, size_t n_gates,
			YaoGate* gate, long counter)
	{
		this->S = &S;
		this->args = &args;
		this->start = start;
		this->end = end;
		this->n_gates = n_gates;
		this->gate = gate;
		this->counter = counter;
		worker.request(*this);
	}

	int run()
	{
		YaoGarbleWire::andrs(*S, *args, start, end, n_gates, gate, counter,
				prng, timers);
		return 0;
	}
};

#endif /* YAO_YAOANDJOB_H_ */
