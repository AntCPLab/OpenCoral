/*
 * YaoWire.cpp
 *
 */

#include "YaoGarbleWire.h"
#include "YaoGate.h"
#include "YaoGarbler.h"
#include "GC/ArgTuples.h"

void YaoGarbleWire::randomize(PRNG& prng)
{
	key = prng.get_doubleword();
#ifdef DEBUG
	//key = YaoGarbler::s().counter << 1;
#endif
	set(key, prng.get_bit());
}

void YaoGarbleWire::set(Key key, bool mask)
{
	key.set_signal(0);
	this->key = key;
	this->mask = mask;
}

void YaoGarbleWire::random()
{
	mask = YaoGarbler::s().prng.get_bit();
	key = 0;
}

void YaoGarbleWire::public_input(bool value)
{
	mask = value;
	key = 0;
}

void YaoGarbleWire::and_(GC::Processor<GC::Secret<YaoGarbleWire> >& processor,
		const vector<int>& args, bool repeat)
{
#ifdef YAO_TIMINGS
	auto& garbler = YaoGarbler::s();
	TimeScope ts(garbler.and_timer), ts2(garbler.and_proc_timer),
			ts3(garbler.and_main_thread_timer);
#endif
	and_multithread(processor, args, repeat);
}

void YaoGarbleWire::and_multithread(GC::Processor<GC::Secret<YaoGarbleWire> >& processor,
		const vector<int>& args, bool repeat)
{
	YaoGarbler& party = YaoGarbler::s();
	int total = processor.check_args(args, 4);
	if (total < party.get_threshold())
	{
		// run in single thread
		and_singlethread(processor, args, repeat);
		return;
	}

	party.and_prepare_timer.start();
	processor.complexity += total;
	SendBuffer& gates = party.gates;
	gates.allocate(total * sizeof(YaoGate));
	int max_gates_per_thread = max(party.get_threshold() / 2,
			(total + party.get_n_worker_threads() - 1) / party.get_n_worker_threads());
	int i_thread = 0, i_gate = 0, start = 0;
	for (size_t j = 0; j < args.size(); j += 4)
	{
		i_gate += args[j];
		size_t end = j + 4;
		if (i_gate >= max_gates_per_thread or end >= args.size())
		{
			YaoGate* gate = (YaoGate*)gates.end();
			gates.skip(i_gate * sizeof(YaoGate));
			party.timers["Dispatch"].start();
			party.and_jobs[i_thread++]->dispatch(processor.S, args, start, end,
					i_gate, gate, party.get_gate_id(), repeat);
			party.timers["Dispatch"].stop();
			party.counter += i_gate;
			i_gate = 0;
			start = end;
		}
	}
	party.and_prepare_timer.stop();
	party.and_wait_timer.start();
	for (int i = 0; i < i_thread; i++)
		party.and_jobs[i]->worker.done();
	party.and_wait_timer.stop();
}

void YaoGarbleWire::and_singlethread(GC::Processor<GC::Secret<YaoGarbleWire> >& processor,
		const vector<int>& args, bool repeat)
{
	int total_ands = processor.check_args(args, 4);
	if (total_ands < 10)
		return processor.andrs(args);
	processor.complexity += total_ands;
	size_t n_args = args.size();
	auto& garbler = YaoGarbler::s();
	SendBuffer& gates = garbler.gates;
	YaoGate* gate = (YaoGate*)gates.allocate_and_skip(total_ands * sizeof(YaoGate));
	long counter = garbler.get_gate_id();
	and_(processor.S, args, 0, n_args, total_ands, gate, counter,
			garbler.prng, garbler.timers, repeat, garbler);
	garbler.counter += counter - garbler.get_gate_id();
}

void YaoGarbleWire::and_(GC::Memory<GC::Secret<YaoGarbleWire> >& S,
		const vector<int>& args, size_t start, size_t end, size_t total_ands,
		YaoGate* gate, long& counter, PRNG& prng, map<string, Timer>& timers,
		bool repeat, YaoGarbler& garbler)
{
	(void)timers;
	Key* labels;
	Key* hashes;
	vector<Key> label_vec, hash_vec;
	size_t n_hashes = 4 * total_ands;
	Key label_arr[400], hash_arr[400];
	if (total_ands < 100)
	{
		labels = label_arr;
		hashes = hash_arr;
	}
	else
	{
		label_vec.resize(n_hashes);
		hash_vec.resize(n_hashes);
		labels = label_vec.data();
		hashes = hash_vec.data();
	}
	//timers["Hash input"].start();
	const Key& delta = garbler.get_delta();
	size_t i_label = 0;
	for (size_t i = start; i < end; i += 4)
	{
		for (int k = 0; k < args[i]; k++)
		{
			auto& left_wire = S[args[i + 2]].get_reg(k);
			const Key& right_key = S[args[i + 3]].get_reg(repeat ? 0 : k).key;
			counter++;
			for (int i = 0; i < 2; i++)
				for (int j = 0; j < 2; j++)
					labels[i_label++] = YaoGate::E_input(
							left_wire.key ^ (i ? delta : 0),
							right_key ^ (j ? delta : 0), counter);
		}
	}
	//timers["Hash input"].stop();
	//timers["Hashing"].start();
	MMO& mmo = garbler.mmo;
	size_t i;
	for (i = 0; i + 8 <= n_hashes; i += 8)
		mmo.hash<8>(&hashes[i], &labels[i]);
	for (; i < n_hashes; i++)
		hashes[i] = mmo.hash(labels[i]);
	//timers["Hashing"].stop();
	//timers["Garbling"].start();
	size_t i_hash = 0;
	for (size_t i = start; i < end; i += 4)
	{
		//timers["Outer ref"].start();
		auto& out = S[args[i + 1]];
		//timers["Outer ref"].stop();
		//timers["Resizing"].start();
		out.resize_regs(args[i]);
		//timers["Resizing"].stop();
		int n = args[i];
		for (int k = 0; k < n; k++)
		{
			YaoGarbleWire& right_wire = S[args[i + 3]].get_reg(repeat ? 0 : k);
			//timers["Inner ref"].start();
			auto& left_wire = S[args[i + 2]].get_reg(k);
			//timers["Inner ref"].stop();
			//timers["Randomizing"].start();
			out.get_reg(k).randomize(prng);
			//timers["Randomizing"].stop();
			//timers["Gate computation"].start();
			(gate++)->garble(out.get_reg(k), &hashes[i_hash], left_wire.mask,
					right_wire.mask, 0x0001, garbler.get_delta());
			//timers["Gate computation"].stop();
			i_hash += 4;
		}
	}
	//timers["Garbling"].stop();
}


void YaoGarbleWire::inputb(GC::Processor<GC::Secret<YaoGarbleWire>>& processor,
        const vector<int>& args)
{
	InputArgList a(args);
	int n_evaluator_bits = 0;
	auto& garbler = YaoGarbler::s();
	bool interactive = garbler.n_interactive_inputs_from_me(a) > 0;
	for (auto x : a)
	{
		auto& dest = processor.S[x.dest];
		dest.resize_regs(x.n_bits);
		if (x.from == 0)
		{
			long long input = processor.get_input(x.n_bits, interactive);
			for (auto& reg : dest.get_regs())
			{
				reg.public_input(input & 1);
				input >>= 1;
			}
		}
		else
		{
			n_evaluator_bits += x.n_bits;
		}
	}

	if (interactive)
	    cout << "Thank you";

	garbler.receiver_input_keys.push_back({});

	for (auto x : a)
	{
		if (x.from == 1)
		{
			for (auto& reg : processor.S[x.dest].get_regs())
			{
				reg.set(garbler.prng.get_doubleword(), 0);
				garbler.receiver_input_keys.back().push_back(reg.key);
			}
		}
	}
}

inline void YaoGarbler::store_gate(const YaoGate& gate)
{
	gates.serialize(gate);
}

void YaoGarbleWire::op(const YaoGarbleWire& left, const YaoGarbleWire& right,
		Function func)
{
	auto& garbler = YaoGarbler::s();
	randomize(garbler.prng);
	YaoGarbler::s().counter++;
	YaoGate gate(*this, left, right, func);
	YaoGarbler::s().store_gate(gate);
}

void YaoGarbleWire::XOR(const YaoGarbleWire& left, const YaoGarbleWire& right)
{
	mask = left.mask ^ right.mask;
	key = left.key ^ right.key;
}

char YaoGarbleWire::get_output()
{
	YaoGarbler::s().output_masks.push_back(mask);
	return -1;
}
